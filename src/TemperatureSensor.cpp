#include "Peripherals.h"
#include "Settings.h"
#include "TemperatureSensor.h"

#include <Arduino.h>

TemperatureSensor::TemperatureSensor(const Settings& settings)
    : _settings(settings)
{}

void TemperatureSensor::task()
{
    if (_lastUpdate == 0 || millis() - _lastUpdate >= UpdateIntervalMs) {
        _lastUpdate = millis();
        Peripherals::Sensors::MainTemperature::update();
    }
}

int16_t TemperatureSensor::read() const
{
    return Peripherals::Sensors::MainTemperature::lastReading()
        + _settings.data.HeatingController.TempCorrection * 10;
}