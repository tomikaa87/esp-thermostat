#pragma once

#include "Config.h"
#include "HeatingZone.h"

#include <CoreApplication.h>
#include <Logger.h>
#include <SettingsHandler.h>

#include <network/MQTT/MqttVariable.h>

#include <array>
#include <cstdint>

class ApplicationConfig;

class FurnaceController
{
public:
    explicit FurnaceController(
        CoreApplication& application,
        const ApplicationConfig& appConfig
    );

    void task(uint32_t deltaMillis);

private:
    const ApplicationConfig& _appConfig;
    CoreApplication& _app;
    
    struct Settings
    {
        bool masterEnable{ false };
        bool energyOptimizerEnabled{ true };
    };

    Setting<Settings> _settings;

    Logger _log{ "FurnaceController" };
    std::array<HeatingZone, Config::ZoneCount> _zones;
    uint32_t _mqttUpdateTimer{};
    bool _relayOutputActive{ false };

    std::string _topicPrefix;
    MqttVariable<int> _masterSwitch;
    MqttVariable<int> _energyOptimizerSwitch;
    MqttVariable<int> _callingForHeatingState;

    void setupRelayOutput() const;
    void setRelayOutputActive(bool active);

    void setupMqttComponentConfigs();
    void setupMqttChangeHandlers();
    void updateMqtt();
};