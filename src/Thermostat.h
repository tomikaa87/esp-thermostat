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
    Blynk _blynk;

    static constexpr auto SlowLoopUpdateIntervalMs = 500;
    uint32_t _lastSlowLoopUpdate = 0;

    struct Mqtt {
        explicit Mqtt(CoreApplication& app)
            : activeTemp(PSTR("thermostat/temp/active"), PSTR("thermostat/temp/active/set"), app.mqttClient())
            , currentTemp(PSTR("thermostat/temp/current"), app.mqttClient())
            , daytimeTemp(PSTR("thermostat/temp/daytime"), PSTR("thermostat/temp/daytime/set"), app.mqttClient())
            , nightTimeTemp(PSTR("thermostat/temp/nightTime"), PSTR("thermostat/temp/nightTime/set"), app.mqttClient())
            , boostRemainingSecs(PSTR("thermostat/boost/remainingSecs"), app.mqttClient())
            , boostActive(PSTR("thermostat/boost/active"), PSTR("thermostat/boost/active/set"), app.mqttClient())
            , heatingActive(PSTR("thermostat/heating/active"), app.mqttClient())
            , heatingMode(PSTR("thermostat/heating/mode"), PSTR("thermostat/heating/mode/set"), app.mqttClient())
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

    void updateBlynk();

    void setupMqtt();
    void updateMqtt();
};