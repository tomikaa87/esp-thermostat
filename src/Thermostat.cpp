#include "display/Display.h"
#include "Extras.h"
#include "Settings.h"
#include "Thermostat.h"

#include <Arduino.h>

#include <sstream>

Thermostat::Thermostat(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _settings(_coreApplication.settings())
    , _temperatureSensor(_settings)
    , _heatingController(_settings, _coreApplication.systemClock(), _temperatureSensor)
    , _ui(_settings, _coreApplication.systemClock(), _keypad, _heatingController, _temperatureSensor)
#ifdef IOT_ENABLE_BLYNK
    , _blynk(_coreApplication.blynkHandler(), _heatingController, _ui, _settings)
#endif
#ifdef IOT_ENABLE_MQTT
    , _mqtt(_coreApplication)
    , _mqttAccessory(_coreApplication)
#endif
{
#ifdef IOT_ENABLE_MQTT
    if (_appConfig.mqtt.enabled) {
        setupMqtt();

        _coreApplication.setMqttUpdateHandler([this] {
            updateMqtt();
        });
    }
#endif

#ifdef IOT_ENABLE_BLYNK
    if (!_settings.data.Scheduler.DisableBlynk) {
        _coreApplication.setBlynkUpdateHandler([this] {
            updateBlynk();
        });
    }
#endif
}

void Thermostat::task()
{
    _coreApplication.task();

#ifdef IOT_ENABLE_BLYNK
    if (!_settings.data.Scheduler.DisableBlynk) {
        _blynk.task();
    }
#endif

    _ui.task();
    _temperatureSensor.task();

    // Slow loop
    if (_lastSlowLoopUpdate == 0 || millis() - _lastSlowLoopUpdate >= SlowLoopUpdateIntervalMs) {
        _lastSlowLoopUpdate = millis();
        _heatingController.task();
        _ui.update();
    }
}

#ifdef IOT_ENABLE_BLYNK
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
#endif

#ifdef IOT_ENABLE_MQTT
void Thermostat::setupMqtt()
{
    _mqtt.activeTemp.setChangedHandler([this](const float v) {
        _heatingController.setTargetTemp(v * 10);
    });

    _mqtt.boostActive.setChangedHandler([this](const bool v) {
        if (v) {
            if (!_heatingController.isBoostActive()) {
                _heatingController.activateBoost();
            } else {
                _heatingController.extendBoost();
            }
        } else {
            _heatingController.deactivateBoost();
        }
    });

    _mqtt.daytimeTemp.setChangedHandler([this](const float v) {
        _heatingController.setDaytimeTemp(v * 10);
    });

    _mqtt.heatingMode.setChangedHandler([this](const int v) {
        if (
            v < static_cast<int>(HeatingController::Mode::_First)
            || v > static_cast<int>(HeatingController::Mode::_Last)
        ) {
            return;
        }

        _heatingController.setMode(static_cast<HeatingController::Mode>(v));
    });

    _mqtt.nightTimeTemp.setChangedHandler([this](const float v) {
        _heatingController.setNightTimeTemp(v * 10);
    });

    //
    // HVAC accessory for Home Assistant
    //

    {
        std::stringstream config;

        config << '{';
        config << Extras::pgmToStdString(PSTR(R"("icon":"mdi:sun-thermometer")"));
        config << Extras::pgmToStdString(PSTR(R"(,"name":"Thermostat")"));
        config << Extras::pgmToStdString(PSTR(R"(,"object_id":"thermostat")"));
        config << Extras::pgmToStdString(PSTR(R"(,"unique_id":"thermostat")"));
        config << Extras::pgmToStdString(PSTR(R"(,"max_temp":30)"));
        config << Extras::pgmToStdString(PSTR(R"(,"min_temp":10)"));
        config << Extras::pgmToStdString(PSTR(R"(,"current_temperature_topic":"thermostat/temp/current")"));
        config << Extras::pgmToStdString(PSTR(R"(,"mode_command_topic":"thermostat/hvac_mode/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"mode_state_topic":"thermostat/hvac_mode")"));
        config << Extras::pgmToStdString(PSTR(R"(,"modes":["auto","heat","off"])"));
        config << Extras::pgmToStdString(PSTR(R"(,"precision":0.1)"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_command_topic":"thermostat/temp/active/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_state_topic":"thermostat/temp/active")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_high_command_topic":"thermostat/temp/daytime/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_high_state_topic":"thermostat/temp/daytime")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_low_command_topic":"thermostat/temp/nightTime/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_low_state_topic":"thermostat/temp/nightTime")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temperature_unit":"C")"));
        config << Extras::pgmToStdString(PSTR(R"(,"temp_step":0.5)"));
        config << '}';

        _coreApplication.mqttClient().publish(
            PSTR("homeassistant/climate/thermostat/config"),
            config.str(),
            false
        );
    }

    {
        std::stringstream config;

        config << '{';
        config << Extras::pgmToStdString(PSTR(R"("icon":"mdi:radiator")"));
        config << Extras::pgmToStdString(PSTR(R"(,"name":"Activate Boost")"));
        config << Extras::pgmToStdString(PSTR(R"(,"object_id":"thermostat_boost_activate")"));
        config << Extras::pgmToStdString(PSTR(R"(,"unique_id":"thermostat_boost_activate")"));
        config << Extras::pgmToStdString(PSTR(R"(,"command_topic":"thermostat/boost/active/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"payload_press":"1")"));
        config << '}';

        _coreApplication.mqttClient().publish(
            PSTR("homeassistant/button/thermostat_boost_activate/config"),
            config.str(),
            false
        );
    }

    {
        std::stringstream config;

        config << '{';
        config << Extras::pgmToStdString(PSTR(R"("icon":"mdi:radiator-off")"));
        config << Extras::pgmToStdString(PSTR(R"(,"name":"Deactivate Boost")"));
        config << Extras::pgmToStdString(PSTR(R"(,"object_id":"thermostat_boost_deactivate")"));
        config << Extras::pgmToStdString(PSTR(R"(,"unique_id":"thermostat_boost_deactivate")"));
        config << Extras::pgmToStdString(PSTR(R"(,"command_topic":"thermostat/boost/active/set")"));
        config << Extras::pgmToStdString(PSTR(R"(,"payload_press":"0")"));
        config << '}';

        _coreApplication.mqttClient().publish(
            PSTR("homeassistant/button/thermostat_boost_deactivate/config"),
            config.str(),
            false
        );
    }

    {
        std::stringstream config;

        config << '{';
        config << Extras::pgmToStdString(PSTR(R"("icon":"mdi:timer")"));
        config << Extras::pgmToStdString(PSTR(R"(,"name":"Boost Remaining")"));
        config << Extras::pgmToStdString(PSTR(R"(,"object_id":"thermostat_boost_remaining")"));
        config << Extras::pgmToStdString(PSTR(R"(,"unique_id":"thermostat_boost_remaining")"));
        config << Extras::pgmToStdString(PSTR(R"(,"state_topic":"thermostat/boost/remainingSecs")"));
        config << Extras::pgmToStdString(PSTR(R"(,"unit_of_measurement":"s")"));
        config << '}';

        _coreApplication.mqttClient().publish(
            PSTR("homeassistant/sensor/thermostat_boost_remaining/config"),
            config.str(),
            false
        );
    }

    _mqttAccessory.hvacMode.setChangedHandler([this](const std::string& mode) {
        if (mode == "off") {
            _heatingController.setMode(HeatingController::Mode::Off);
        } else if (mode == "heat") {
            if (!_heatingController.isBoostActive()) {
                _heatingController.activateBoost();
            }
        } else if (mode == "auto") {
            _heatingController.setMode(HeatingController::Mode::Normal);
        }
    });
}

void Thermostat::updateMqtt()
{
    _mqtt.activeTemp = _heatingController.targetTemp() / 10.f;
    _mqtt.boostActive = _heatingController.isBoostActive();
    _mqtt.boostRemainingSecs = _heatingController.boostRemaining();
    _mqtt.currentTemp = _heatingController.currentTemp() / 10.f;
    _mqtt.daytimeTemp = _heatingController.daytimeTemp() / 10.f;
    _mqtt.heatingActive = _heatingController.isActive();
    _mqtt.heatingMode = static_cast<int>(_heatingController.mode());
    _mqtt.nightTimeTemp = _heatingController.nightTimeTemp() / 10.f;

    _mqttAccessory.hvacMode = [this] {
        switch (_heatingController.mode()) {
            case HeatingController::Mode::Boost:
                return "heat";

            case HeatingController::Mode::Normal:
                return "auto";

            default:
                return "off";
        }
    }();
}
#endif