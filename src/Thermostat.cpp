#include "display/Display.h"
#include "FirmwareVersion.h"
#include "Peripherals.h"
#include "PrivateConfig.h"
#include "Settings.h"
#include "Thermostat.h"

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>

Thermostat::Thermostat()
    : _heatingController(_settings, _systemClock)
    , _ntpClient(_systemClock)
    , _blynk(PrivateConfig::BlynkAppToken, _heatingController)
    , _ui(_settings, _systemClock, _keypad, _heatingController)
    , _otaUpdater("http://tomikaa.noip.me:8001/esp-thermostat/update", _systemClock)
{
    _log.info("initializing, firmware version: %d.%d.%d", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_PATCH);

    // Disable ASE by default to avoid unnecessary wearing when settings are not changed
    Peripherals::Storage::EERAM::StatusReg sr;
    sr.value = 0;
    Peripherals::Storage::EERAM::setStatus(sr);

    auto eeramStatus = Peripherals::Storage::EERAM::getStatus();
    _log.info("EERAM status: AM=%u, BP=%u, ASE=%u, EVENT=%u\n",
        eeramStatus.am, eeramStatus.bp, eeramStatus.ase, eeramStatus.event
    );

    _updateCheckTimer = millis();
}

void Thermostat::task()
{
    // connectToWiFi();

    _systemClock.task();
    _ntpClient.task();
    _blynk.task();
    _ui.task();
    _otaUpdater.task();

    // Temperature sensor loop
    if (_lastTempSensorUpdate == 0 || millis() - _lastTempSensorUpdate >= TempSensorUpdateIntervalMs) {
        _lastTempSensorUpdate = millis();
        Peripherals::Sensors::MainTemperature::update();
    }

    // Slow loop
    if (_lastSlowLoopUpdate == 0 || millis() - _lastSlowLoopUpdate >= SlowLoopUpdateIntervalMs) {
        _lastSlowLoopUpdate = millis();
        _heatingController.task();
        _ui.update();

        if (!_updateChecked && WiFi.isConnected() && millis() - _updateCheckTimer >= 5000) {
            _updateChecked = true;
            _otaUpdater.forceUpdate();
        }
    }

    // Blynk update loop
    if (_lastBlynkUpdate == 0 || millis() - _lastBlynkUpdate >= BlynkUpdateIntervalMs) {
        _lastBlynkUpdate = millis();
        updateBlynk();
    }
}

void ICACHE_RAM_ATTR Thermostat::epochTimerIsr()
{
    _systemClock.timerIsr();
}

void Thermostat::connectToWiFi()
{
    if (WiFi.isConnected()) {
        if (_wifiConnecting) {
            _log.info("connected to WiFi AP");
            _wifiConnecting = false;
        }
        return;
    }

    if (_wifiConnecting) {
        return;
    }

    _wifiConnecting = true;

    _log.info("connecting to WiFi AP: %s", PrivateConfig::WiFiSSID);

    WiFi.begin(PrivateConfig::WiFiSSID, PrivateConfig::WiFiPassword);
}

void Thermostat::updateBlynk()
{
    _blynk.updateActiveTemperature(_heatingController.targetTemp() / 10.f);
    _blynk.updateBoostRemaining(_heatingController.boostRemaining());
    _blynk.updateCurrentTemperature(_heatingController.currentTemp() / 10.f);
    _blynk.updateDaytimeTemperature(_heatingController.daytimeTemp() / 10.f);
    _blynk.updateIsBoostActive(_heatingController.isBoostActive());
    _blynk.updateIsHeatingActive(_heatingController.isActive());
    _blynk.updateMode(static_cast<uint8_t>(_heatingController.mode()));
    _blynk.updateNightTimeTemperature(_heatingController.nightTimeTemp() / 10.f);

    const auto nt = _heatingController.nextTransition();
    _blynk.updateNextSwitch(static_cast<uint8_t>(nt.state), nt.weekday, nt.hour, nt.minute);
}