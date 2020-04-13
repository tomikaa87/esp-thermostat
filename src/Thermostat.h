#pragma once

#include "SystemClock.h"
#include "HeatingController.h"
#include "Keypad.h"
#include "Logger.h"
#include "network/BlynkHandler.h"
#include "network/NtpClient.h"
#include "ui/Ui.h"

class Thermostat
{
public:
    Thermostat();

    void task();
    void epochTimerIsr();

private:
    SystemClock _systemClock;
    Logger _log{ "Thermostat" };
    HeatingController _heatingController;
    NtpClient _ntpClient;
    BlynkHandler _blynk;
    Keypad _keypad;
    Ui _ui;

    bool _wifiConnecting = false;

    static constexpr auto SlowLoopUpdateIntervalMs = 500;
    std::time_t _lastSlowLoopUpdate = 0;

    static constexpr auto BlynkUpdateIntervalMs = 1000;
    std::time_t _lastBlynkUpdate = 0;

    void connectToWiFi();
    void updateBlynk();
};