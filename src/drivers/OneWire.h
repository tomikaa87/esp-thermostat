#pragma once

#include <Arduino.h>
#include <cstdint>

namespace Drivers
{

namespace Detail
{
    namespace OneWireImpl
    {
        uint8_t reset(int pin);

        void writeBit(int pin, uint8_t b);
        void writeByte(int pin, uint8_t b);

        uint8_t readBit(int pin);
        uint8_t readByte(int pin);

        void busLow(int pin);
        void busHigh(int pin);
        void busFloat(int pin);
        uint8_t busRead(int pin);
    }
}

template <int Pin>
class OneWire
{
public:
    OneWire() = delete;

    static uint8_t reset()
    {
        return Detail::OneWireImpl::reset(Pin);
    }

    static void writeBit(const uint8_t b)
    {
        Detail::OneWireImpl::writeBit(Pin, b);
    }

    static void writeByte(const uint8_t b)
    {
        Detail::OneWireImpl::writeByte(Pin, b);
    }

    static uint8_t readBit()
    {
        return Detail::OneWireImpl::readBit(Pin);
    }

    static uint8_t readByte()
    {
        return Detail::OneWireImpl::readByte(Pin);
    }
};

}
