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

    void updateBlynk();
};