#pragma once

#include <cstdint>

namespace Drivers
{

class EERAM
{
public:
    static constexpr auto A1 = 0;
    static constexpr auto A2 = 0;

    enum class Register : uint8_t
    {
        Status = 0,
        Command = 0x55
    };

    enum class Command : uint8_t
    {
        SoftwareStore = 0b00110011,
        SoftwareRecall = 0b11011101
    };

    union StatusReg
    {
        struct
        {
            uint8_t event : 1;
            uint8_t ase : 1;
            uint8_t bp : 3;
            uint8_t _reserved : 2;
            uint8_t am : 1;
        } __attribute__((packed));
        uint8_t value;
    } __attribute__((packed));

    EERAM() = delete;

    static uint8_t read(uint16_t address, uint8_t* buffer, uint16_t length);
    static void write(uint16_t address, const uint8_t* data, uint16_t length);

    static uint8_t readControlReg(Register reg);
    static void writeControlReg(Register reg, uint8_t value);

    static StatusReg getStatus();
    static void setStatus(StatusReg sr);

    static void executeCommand(Command cmd);
};

}