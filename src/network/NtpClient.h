#pragma once

#include "Logger.h"

#include <Arduino.h>
#include <WiFiUdp.h>

#include <ctime>
#include <functional>
#include <memory>

class Clock;

class NtpClient
{
public:
    static constexpr auto Server = "europe.pool.ntp.org";
    static constexpr auto UpdateInterval = 24 * 60 * 60;
    static constexpr auto NtpPacketSize = 48;
    static constexpr auto SeventyYears = 2208988800ul;
    static constexpr auto NtpPort = 123;
    static constexpr auto NtpDefaultLocalPort = 1337;

    NtpClient(Clock& clock);

    void task();

    using UpdatedHandler = std::function<void(std::time_t)>;
    void setUpdatedCallback(UpdatedHandler&& handler);

private:
    Clock& _clock;
    Logger _log{ "NtpClient" };
    UpdatedHandler _updatedHandler;
    std::unique_ptr<WiFiUDP> _socket = nullptr;
    std::time_t _lastUpdate = 0;
    uint32_t _sendTimestamp = 0;

    enum class State
    {
        Idle,
        SendPacket,
        WaitResponse
    };
    State _state = State::Idle;

    void sendPacket();
};