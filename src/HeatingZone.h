#pragma once

#include "network/MQTT/MqttVariable.h"

#include <HeatingZoneController.h>

class CoreApplication;

class HeatingZone
{
public:
    explicit HeatingZone(
        unsigned index,
        CoreApplication& app
    );

private:
    CoreApplication& _app;
    HeatingZoneController _controller;
    const std::string _topicPrefix;
    const unsigned _index;

    MqttVariable<int> _mode;
    MqttVariable<float> _targetTemperature;
    MqttVariable<float> _currentTemperature;
    MqttVariable<float> _lowTargetTemperature;
    MqttVariable<float> _highTargetTemperature;
    MqttVariable<std::string> _action;
    MqttVariable<std::string> _presetMode;

    void setupMqtt();

    std::string makeTopic(PGM_P prependToPrefix, PGM_P appendToPrefix) const;
    std::string makeClimateConfig() const;
    std::string makeButtonConfig(
        PGM_P icon,
        PGM_P name,
        PGM_P id,
        PGM_P commandTopic,
        PGM_P pressPayload
    ) const;
};