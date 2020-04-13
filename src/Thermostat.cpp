#include "display/Display.h"
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
    : _log{ "Thermostat" }
    , _heatingController{ _clock }
    , _ntpClient{ _clock }
    , _blynk{ PrivateConfig::BlynkAppToken, _heatingController }
    , _ui{ _clock, _keypad, _heatingController }
{
    connectToWiFi();

    Peripherals::Sensors::MainTemperature::update();

    auto eeramStatus = Peripherals::Storage::EERAM::getStatus();
    _log.debugf("EERAM status: AM=%u, BP=%u, ASE=%u, EVENT=%u\n",
        eeramStatus.am, eeramStatus.bp, eeramStatus.ase, eeramStatus.event
    );

    _log.info("initializing Display");
    Display::init();

    _log.info("loading settings...");
    settings_init(
        [](const uint8_t address) {
            uint8_t data;
            if (Peripherals::Storage::EERAM::read(address, &data, 1) != 1)
                Serial.printf("EERAM read failure at %02xh\n", address);
            Serial.printf("RAM[%02xh] -> %02xh\n", address, data);
            return data;
        },
        [](const uint8_t address, const uint8_t data) {
            Serial.printf("RAM[%02xh] <- %02xh\n", address, data);
            Peripherals::Storage::EERAM::write(address, &data, 1);
        }
    );
    settings_load();
}

void Thermostat::task()
{
    _clock.task();
    _ntpClient.task();
    _blynk.task();
    _ui.task();

    // Slow loop
    if (millis() - _lastSlowLoopUpdate >= SlowLoopUpdateIntervalMs) {
        _lastSlowLoopUpdate = millis();
        Peripherals::Sensors::MainTemperature::update();
        _heatingController.task();
        _ui.update();
    }

    // Blynk update loop
    if (millis() - _lastBlynkUpdate >= BlynkUpdateIntervalMs) {
        _lastBlynkUpdate = millis();
        updateBlynk();
    }
}

void Thermostat::epochTimerIsr()
{
    _clock.timerIsr();
}

void Thermostat::connectToWiFi()
{
    _log.infof("connecting to WiFi AP: %s", PrivateConfig::WiFiSSID);

    WiFi.mode(WIFI_STA);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    WiFi.setOutputPower(20.5);

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(PrivateConfig::WiFiSSID, PrivateConfig::WiFiPassword);
        // if (pass && strlen(pass)) {
        //     WiFi.begin(ssid, pass);
        // } else {
        //     WiFi.begin(ssid);
        // }
    }

    while (WiFi.status() != WL_CONNECTED) {
        // TODO avoid using delay, use task() instead
        delay(500);
    }

    _log.info("connected to WiFi AP");
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