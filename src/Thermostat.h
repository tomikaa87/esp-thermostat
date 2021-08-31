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
            : activeTemp("temp/active", app.mqttClient())
            , currentTemp("temp/current", app.mqttClient())
            , daytimeTemp("temp/daytime", app.mqttClient())
            , nightTimeTemp("temp/nightTime", app.mqttClient())
            , boostRemainingSecs("boost/remainingSecs", app.mqttClient())
            , boostActive("boost/active", app.mqttClient())
            , heatingActive("heating/active", app.mqttClient())
            , heatingMode("heating/mode", app.mqttClient())
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
    void updateMqtt();
};