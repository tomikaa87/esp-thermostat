#pragma once

#include "Blynk.h"
#include "HeatingController.h"
#include "Keypad.h"
#include "Settings.h"
#include "TemperatureSensor.h"
#include "network/BlynkHandler.h"
#include "ui/Ui.h"

#include <CoreApplication.h>
#include <Logger.h>
#include <network/MQTT/MqttVariable.h>

class Thermostat
{
public:
    Thermostat(const ApplicationConfig& appConfig);

    void task();

private:
    CoreApplication _coreApplication;
    const ApplicationConfig& _appConfig;
    Settings _settings;
    Logger _log{ "Thermostat" };
    TemperatureSensor _temperatureSensor;
    HeatingController _heatingController;
    Keypad _keypad;
    Ui _ui;

#ifdef IOT_ENABLE_BLYNK
    Blynk _blynk;
    void updateBlynk();
#endif

    static constexpr auto SlowLoopUpdateIntervalMs = 500;
    uint32_t _lastSlowLoopUpdate = 0;

    struct Mqtt {
        explicit Mqtt(CoreApplication& app)
            : activeTemp(           PSTR("thermostat/temp/active"),     PSTR("thermostat/temp/active/set"), app.mqttClient())
            , currentTemp(          PSTR("thermostat/temp/current"), app.mqttClient())
            , daytimeTemp(          PSTR("thermostat/temp/daytime"),    PSTR("thermostat/temp/daytime/set"), app.mqttClient())
            , nightTimeTemp(        PSTR("thermostat/temp/nightTime"),  PSTR("thermostat/temp/nightTime/set"), app.mqttClient())
            , boostRemainingSecs(   PSTR("thermostat/boost/remainingSecs"), app.mqttClient())
            , boostActive(          PSTR("thermostat/boost/active"),    PSTR("thermostat/boost/active/set"), app.mqttClient())
            , heatingActive(        PSTR("thermostat/heating/active"), app.mqttClient())
            , heatingMode(          PSTR("thermostat/heating/mode"),    PSTR("thermostat/heating/mode/set"), app.mqttClient())
        {}

        MqttVariable<float> activeTemp;
        MqttVariable<float> currentTemp;
        MqttVariable<float> daytimeTemp;
        MqttVariable<float> nightTimeTemp;
        MqttVariable<int> boostRemainingSecs;
        MqttVariable<bool> boostActive;
        MqttVariable<bool> heatingActive;
        MqttVariable<int> heatingMode;
    } _mqtt;

    struct MqttAccessory {
        explicit MqttAccessory(CoreApplication& app)
            : hvacConfig(PSTR("homeassistant/climate/thermostat/config"), app.mqttClient())
            , hvacMode(PSTR("thermostat/hvac_mode"), PSTR("thermostat/hvac_mode/set"), app.mqttClient())
            , boostActivateButtonConfig(PSTR("homeassistant/button/thermostat_boost_activate/config"), app.mqttClient())
            , boostDeactivateButtonConfig(PSTR("homeassistant/button/thermostat_boost_deactivate/config"), app.mqttClient())
            , boostRemainingSecondsConfig(PSTR("homeassistant/sensor/thermostat_boost_remaining/config"), app.mqttClient())

        {}

        MqttVariable<std::string> hvacConfig;
        MqttVariable<std::string> hvacMode;
        MqttVariable<std::string> boostActivateButtonConfig;
        MqttVariable<std::string> boostDeactivateButtonConfig;
        MqttVariable<std::string> boostRemainingSecondsConfig;
    } _mqttAccessory;

    void setupMqtt();
    void updateMqtt();
};