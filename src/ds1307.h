#pragma once

#include <cstdint>
#include <functional>

class DS1307
{
public:
    static constexpr auto Address = 0x68;

    using WriteRegFunc = std::function<void(uint8_t reg, uint8_t value)>;
    using ReadRegFunc = std::function<uint8_t(uint8_t reg)>;

    DS1307(WriteRegFunc&& writeReg, ReadRegFunc&& readReg);

    uint8_t getSeconds() const;
    void setSeconds(uint8_t value) const;

    uint8_t getMinutes() const;
    void setMinutes(uint8_t value) const;

    uint8_t getHours() const;
    void setHours(uint8_t value) const;

    uint8_t getDayOfWeek() const;
    void setDayOfWeek(uint8_t value) const;

    uint8_t getDate() const;
    void setDate(uint8_t value) const;

    uint8_t getMonth() const;
    void setMonth(uint8_t value) const;

    uint8_t getYear() const;
    void setYear(uint8_t value) const;

    uint8_t readRam(uint8_t address) const;
    void writeRam(uint8_t address, uint8_t data) const;

private:
    WriteRegFunc _writeReg;
    ReadRegFunc _readReg;

    static constexpr uint8_t toBcd(uint8_t value);
    static constexpr uint8_t toDec(uint8_t value);
};