#include "ds1307.h"

namespace Registers
{
    constexpr auto Seconds      = 0x00u;
    constexpr auto Minutes      = 0x01u;
    constexpr auto Hours        = 0x02u;
    constexpr auto DayOfWeek    = 0x03u;
    constexpr auto Date         = 0x04u;
    constexpr auto Month        = 0x05u;
    constexpr auto Year         = 0x06u;
    constexpr auto Control      = 0x07u;
    constexpr auto RamStart     = 0x08u;
    constexpr auto RamEnd       = 0x3fu;
}

namespace Control
{
    constexpr auto RS0          = 0x01u;
    constexpr auto RS1          = 0x02u;
    constexpr auto SQWE         = 0x10u;
    constexpr auto OUT          = 0x80u;
}

DS1307::DS1307(WriteRegFunc&& writeReg, ReadRegFunc&& readReg)
    : _writeReg{ std::move(writeReg) }
    , _readReg{ std::move(readReg) }
{}

uint8_t DS1307::getSeconds() const
{
    // Ignore CH bit
    return toDec(_readReg(Registers::Seconds) & 0x7Fu);
}

void DS1307::setSeconds(const uint8_t value) const
{
    // Always reset CH bit
    _writeReg(Registers::Seconds, toBcd(value) & 0x7Fu);
}

uint8_t DS1307::getMinutes() const
{
    return toDec(_readReg(Registers::Minutes));
}

void DS1307::setMinutes(const uint8_t value) const
{
    _writeReg(Registers::Minutes, toBcd(value));
}

uint8_t DS1307::getHours() const
{
    // Ignore 12/24 mode bits
    return toDec(_readReg(Registers::Hours) & 0x3Fu);
}

void DS1307::setHours(const uint8_t value) const
{
    // Always reset 12/24 mode bits
    _writeReg(Registers::Hours, toBcd(value) & 0x3Fu);
}

uint8_t DS1307::getDayOfWeek() const
{
    return _readReg(Registers::DayOfWeek);
}

void DS1307::setDayOfWeek(const uint8_t value) const
{
    _writeReg(Registers::DayOfWeek, value & 0x07u);
}

uint8_t DS1307::getDate() const
{
    return toDec(_readReg(Registers::Date));
}

void DS1307::setDate(const uint8_t value) const
{
    _writeReg(Registers::Date, toBcd(value));
}

uint8_t DS1307::getMonth() const
{
    return toDec(_readReg(Registers::Month));
}

void DS1307::setMonth(const uint8_t value) const
{
    _writeReg(Registers::Month, toBcd(value));
}

uint8_t DS1307::getYear() const
{
    return toDec(_readReg(Registers::Year));
}

void DS1307::setYear(const uint8_t value) const
{
    _writeReg(Registers::Year, toBcd(value));
}

constexpr uint8_t DS1307::toBcd(const uint8_t value)
{
    return value % 10 | (value / 10) << 4;
}

constexpr uint8_t DS1307::toDec(const uint8_t value)
{
    return (value & 0x0F) + 10 * (value >> 4);
}

uint8_t DS1307::readRam(const uint8_t address) const
{
    const auto reg = Registers::RamStart + address;

    if (reg > Registers::RamEnd)
        return 0;
    
    return _readReg(reg);
}

void DS1307::writeRam(const uint8_t address, const uint8_t data) const
{
    const auto reg = Registers::RamStart + address;

    if (reg > Registers::RamEnd)
        return;

    _writeReg(reg, data);
}