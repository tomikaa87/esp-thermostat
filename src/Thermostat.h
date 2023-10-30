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
            : activeTemp(           PSTR("furnace/temp/active"),     PSTR("furnace/temp/active/set"), app.mqttClient())
            , currentTemp(          PSTR("furnace/temp/current"), app.mqttClient())
            , daytimeTemp(          PSTR("furnace/temp/daytime"),    PSTR("furnace/temp/daytime/set"), app.mqttClient())
            , nightTimeTemp(        PSTR("furnace/temp/nightTime"),  PSTR("furnace/temp/nightTime/set"), app.mqttClient())
            , remoteTemp(           PSTR("furnace/temp/remote"),     PSTR("furnace/temp/remote/set"), app.mqttClient())
            , internalTemp(         PSTR("furnace/temp/internal"), app.mqttClient())
            , boostRemainingSecs(   PSTR("furnace/boost/remainingSecs"), app.mqttClient())
            , boostActive(          PSTR("furnace/boost/active"),    PSTR("furnace/boost/active/set"), app.mqttClient())
            , heatingActive(        PSTR("furnace/heating/active"), app.mqttClient())
            , heatingMode(          PSTR("furnace/heating/mode"),    PSTR("furnace/heating/mode/set"), app.mqttClient())
        {}

        MqttVariable<float> activeTemp;
        MqttVariable<float> currentTemp;
        MqttVariable<float> daytimeTemp;
        MqttVariable<float> nightTimeTemp;
        MqttVariable<float> remoteTemp;
        MqttVariable<float> internalTemp;
        MqttVariable<int> boostRemainingSecs;
        MqttVariable<bool> boostActive;
        MqttVariable<bool> heatingActive;
        MqttVariable<int> heatingMode;
    } _mqtt;

    struct MqttAccessory {
        explicit MqttAccessory(CoreApplication& app)
            : hvacMode(PSTR("furnace/hvac_mode"), PSTR("furnace/hvac_mode/set"), app.mqttClient())

        {}

        MqttVariable<std::string> hvacMode;
    } _mqttAccessory;

    void setupMqtt();
    void updateMqtt();
};