#pragma once

#include "Clock.h"
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
    Clock _clock;
    Logger _log;
    HeatingController _heatingController;
    NtpClient _ntpClient;
    BlynkHandler _blynk;
    Keypad _keypad;
    Ui _ui;

    static constexpr auto SlowLoopUpdateIntervalMs = 500;
    std::time_t _lastSlowLoopUpdate = 0;

    static constexpr auto BlynkUpdateIntervalMs = 1000;
    std::time_t _lastBlynkUpdate = 0;

    void connectToWiFi();
    void updateBlynk();
};