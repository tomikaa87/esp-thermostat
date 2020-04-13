#pragma once

#include <cstdint>

#include <Arduino.h>
#include <esp8266_peri.h>

// #define SIMPLE_I2C_DEBUG

namespace Drivers
{

template <int SdaPin, int SclPin, int Frequency>
class SimpleI2C
{
    static constexpr int calculateDelayCount(const int freq)
    {
        return 
            freq <= 50000 ? 38 :
            freq <= 100000 ? 19 :
            freq <= 200000 ? 8 :
            freq <= 300000 ? 3 :
            1;
    }

public:
    static constexpr auto DelayCount = calculateDelayCount(Frequency);

    enum class Operation
    {
        Read,
        Write
    };

    static void init()
    {
        pinMode(SdaPin, INPUT_PULLUP);
        pinMode(SclPin, INPUT_PULLUP);
    }

    static bool start(const uint8_t addr, const Operation op, const bool sendStopOnError = true)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<ST:%x:%d>", addr, op);
#endif

        if (!writeStart()) {
            return false;
        }

        if (!writeByte(((addr << 1) | (op == Operation::Read ? 1 : 0)) & 0xff)) {
            if (sendStopOnError) {
                writeStop();
            }

            return false;
        }

        return true;
    }

    static bool write(const uint8_t* buf, const std::size_t len, const bool sendStopOnError = true)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<W:%u>", len);
#endif

        for (std::size_t i = 0; i < len; ++i) {
            if (!writeByte(buf[i])) {
                if (sendStopOnError) {
                    writeStop();
                }

                return false;
            }
        }

        return true;
    }

    static void end(const bool sendStop = true)
    {
        if (sendStop) {
            writeStop();
        } else {
            sclWalley();
        }

        uint8_t i = 0;
        while (!sdaRead() && i++ < 10) {
            sclWalley();
            busyWait();
        }

#ifdef SIMPLE_I2C_DEBUG
        Serial.println("<E>");
#endif
    }

    static bool write(const uint8_t slaveAddr, const uint8_t* buf, const std::size_t len, const bool sendStop = true)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<WT:%0x:%u>", slaveAddr, len);
#endif

        if (!start(slaveAddr, Operation::Write, sendStop)) {
            return false;
        }

        if (!write(buf, len, sendStop)) {
            return false;
        }

        end(sendStop);

        return true;
    }

    static void read(uint8_t* buf, const std::size_t len, const bool sendNack = true)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<R:%u:%c>", len, sendNack ? 'N' : 'A');
#endif

        for (std::size_t i = 0; i < (len - 1); ++i) {
            buf[i] = readByte(false);
        }
        buf[len - 1] = readByte(sendNack);
    }

    static bool read(const uint8_t slaveAddr, uint8_t* buf, const std::size_t len, const bool sendStop = true)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<RT:%02x:%u>", slaveAddr, len);
#endif
    
        if (!start(slaveAddr, Operation::Read, sendStop)) {
            return false;
        }

        read(buf, len, true);
        end(sendStop);

        return true;
    }

private:
    static inline __attribute__((always_inline)) void sdaLow()
    {
        GPES = (1 << SdaPin);
    }

    static inline __attribute__((always_inline)) void sdaHigh()
    {
        GPEC = (1 << SdaPin);
    }

    static inline __attribute__((always_inline)) bool sdaRead()
    {
        return (GPI & (1 << SdaPin)) != 0;
    }

    static inline __attribute__((always_inline)) void sclLow()
    {
        GPES = (1 << SclPin);
    }

    static inline __attribute__((always_inline)) void sclHigh()
    {
        GPEC = (1 << SclPin);
    }

    static inline __attribute__((always_inline)) bool sclRead()
    {
        return (GPI & (1 << SclPin)) != 0;
    }

    static void sclWalley()
    {
        sclLow();
        busyWait();
        sclHigh();
        busyWait();
    }

    static void busyWait(const int v = DelayCount)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    unsigned int reg;
    for (auto i = 0; i < v; i++) {
        reg = GPI;
    }
    (void)reg;
#pragma GCC diagnostic pop
    }

    static bool writeStart()
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.print("<S>");
#endif

        sclHigh();
        sdaHigh();
        if (!sdaRead()) {
            return false;
        }
        busyWait();
        sdaLow();
        busyWait();
        return true;
    }

    static void writeStop()
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.print("<P>");
#endif

        sclLow();
        sdaLow();
        busyWait();
        sclHigh();
        busyWait();
        sdaHigh();
        busyWait();
    }

    static void writeBit(bool bit)
    {
        sclLow();
        if (bit) {
            sdaHigh();
        } else {
            sdaLow();
        }
        busyWait();
        sclHigh();
        busyWait();
    }

    static bool readBit()
    {
        sclLow();
        sdaHigh();
        busyWait(DelayCount + 2);
        sclHigh();
        const auto bit = sdaRead();
        busyWait();
        return bit;
    }

    static bool writeByte(uint8_t b)
    {
#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<WB:%02x>", b);
#endif

        for (uint8_t bit = 0; bit < 8; ++bit) {
            writeBit(b & 0x80);
            b <<= 1;
        }

        return !readBit(); // NACK/ACK
    }

    static uint8_t readByte(bool nack)
    {
        uint8_t byte = 0;

        for (uint8_t bit = 0; bit < 8; ++bit) {
            byte = (byte << 1) | (readBit() ? 1 : 0);
        }

        writeBit(nack);

#ifdef SIMPLE_I2C_DEBUG
        Serial.printf("<RB:%02x:%c>", byte, nack ? 'N' : 'A');
#endif

        return byte;
    }
};

using I2C = SimpleI2C<4, 5, 400000>;

}