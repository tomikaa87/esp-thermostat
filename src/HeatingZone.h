#pragma once

#include "Settings.h"

#include "network/MQTT/MqttVariable.h"

#include <HeatingZoneController.h>
#include <Logger.h>

class CoreApplication;

class HeatingZone
{
public:
    explicit HeatingZone(
        unsigned index,
        CoreApplication& app
    );

    void task(uint32_t systemClockDeltaMs);

    [[nodiscard]] bool callingForHeating();

    void loadDefaultSettings();

private:
    CoreApplication& _app;
    Logger _log;
    HeatingZoneController::Configuration _controllerConfig{};
    HeatingZoneController::Schedule _controllerSchedule{};
    HeatingZoneController::State _controllerState{};
    HeatingZoneController _controller;
    const std::string _topicPrefix;

    MqttVariable<std::string> _mode;
    MqttVariable<std::string> _targetTemperature;
    MqttVariable<float> _lowTargetTemperature;
    MqttVariable<float> _highTargetTemperature;
    MqttVariable<float> _remoteTemperature;
    MqttVariable<std::string> _action;
    MqttVariable<std::string> _presetMode;
    MqttVariable<int> _boostRemainingSeconds;
    MqttVariable<int> _boostActive;

    uint32_t _mqttUpdateTimer{};

    void setupMqttComponentConfigs();
    void setupMqttChangeHandlers();
    void updateMqtt();

    void onModeChanged(const std::string& mode);
    void onTargetTemperatureChanged(float value);
    void onLowTargetTemperatureChanged(float value);
    void onHighTargetTemperatureChanged(float value);
    void onRemoteTemperatureChanged(float value);
    void onPresetModeChanged(const std::string& value);
    void onBoostActiveChanged(int value);

    std::string makeTopic(PGM_P prependToPrefix, PGM_P appendToPrefix) const;
    std::string makeClimateConfig() const;
    std::string makeButtonConfig(
        PGM_P icon,
        PGM_P name,
        PGM_P id,
        PGM_P commandTopic,
        PGM_P pressPayload
    ) const;
    std::string makeSensorConfig(
        PGM_P icon,
        PGM_P name,
        PGM_P id,
        PGM_P stateTopic,
        PGM_P unit
    ) const;
    std::string makeRemoteTemperatureSensorConfig(
        PGM_P name,
        PGM_P id,
        PGM_P commandTopic,
        PGM_P stateTopic
    ) const;
};