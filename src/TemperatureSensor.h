#pragma once

#include <cstdint>

class Settings;

class TemperatureSensor
{
public:
    static constexpr auto UpdateIntervalMs = 2500;

    explicit TemperatureSensor(const Settings& settings);

    void task();

    int16_t read() const;

private:
    const Settings& _settings;
    uint32_t _lastUpdate = 0;
};