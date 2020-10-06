#include "display/Display.h"
#include "Settings.h"
#include "Thermostat.h"

#include <Arduino.h>

Thermostat::Thermostat(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _settings(_coreApplication.settings())
    , _temperatureSensor(_settings)
    , _heatingController(_settings, _coreApplication.systemClock(), _temperatureSensor)
    , _blynk(_coreApplication.blynkHandler(), _heatingController)
    , _ui(_settings, _coreApplication.systemClock(), _keypad, _heatingController, _temperatureSensor)
{
    _coreApplication.setBlynkUpdateHandler([this] {
        updateBlynk();
    });
}

void Thermostat::task()
{
    _coreApplication.task();
    _blynk.task();
    _ui.task();
    _temperatureSensor.task();

    // Slow loop
    if (_lastSlowLoopUpdate == 0 || millis() - _lastSlowLoopUpdate >= SlowLoopUpdateIntervalMs) {
        _lastSlowLoopUpdate = millis();
        _heatingController.task();
        _ui.update();
    }
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