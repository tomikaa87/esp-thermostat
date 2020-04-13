#pragma once

#include "HeatingController.h"
#include "Keypad.h"
#include "Logger.h"
#include "Settings.h"
#include "SystemClock.h"
#include "network/BlynkHandler.h"
#include "network/NtpClient.h"
#include "network/OtaUpdater.h"
#include "ui/Ui.h"

class Thermostat
{
public:
    Thermostat();

    void task();
    void epochTimerIsr();

private:
    Settings _settings;
    SystemClock _systemClock;
    Logger _log{ "Thermostat" };
    HeatingController _heatingController;
    NtpClient _ntpClient;
    BlynkHandler _blynk;
    Keypad _keypad;
    Ui _ui;
    OtaUpdater _otaUpdater;

    bool _wifiConnecting = false;

    bool _updateChecked = false;
    uint32_t _updateCheckTimer = 0;

    static constexpr auto SlowLoopUpdateIntervalMs = 500;
    uint32_t _lastSlowLoopUpdate = 0;

    static constexpr auto BlynkUpdateIntervalMs = 1000;
    uint32_t _lastBlynkUpdate = 0;

    static constexpr auto TempSensorUpdateIntervalMs = 2500;
    uint32_t _lastTempSensorUpdate = 0;

    void connectToWiFi();
    void updateBlynk();
};