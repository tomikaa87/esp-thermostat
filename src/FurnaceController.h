#pragma once

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
    static constexpr auto ZoneCount = 5;

    explicit FurnaceController(const ApplicationConfig& appConfig);

    void task();

private:
    const ApplicationConfig& _appConfig;
    CoreApplication _app;
    
    struct Settings
    {
        bool masterEnable{ false };
    };

    Setting<Settings> _settings;

    Logger _log{ "FurnaceController" };
    std::array<HeatingZone, ZoneCount> _zones;
    uint32_t _lastTaskMillis{};
    uint32_t _mqttUpdateTimer{};
    bool _relayOutputActive{ false };

    std::string _topicPrefix;
    MqttVariable<int> _masterSwitch;
    MqttVariable<int> _callingForHeatingState;

    void setupRelayOutput() const;
    void setRelayOutputActive(bool active);

    void setupMqttComponentConfigs();
    void setupMqttChangeHandlers();
    void updateMqtt();
};