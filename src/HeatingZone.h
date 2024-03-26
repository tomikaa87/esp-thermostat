#pragma once

#include "network/MQTT/MqttVariable.h"

#include <SettingsHandler.h>
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

    void handleFurnaceHeatingChanged(bool heating);

private:
    unsigned _index{};
    CoreApplication& _app;
    Logger _log;
    HeatingZoneController::Configuration _controllerConfig;
    HeatingZoneController::Schedule _controllerSchedule;
    Setting<HeatingZoneController::State> _stateSetting;
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
    MqttVariable<int> _windowState;
    MqttVariable<int> _openWindowLockoutRemaining;

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
    void onWindowStateChanged(bool open);
};