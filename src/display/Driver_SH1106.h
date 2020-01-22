#pragma once

#include <stdint.h>

namespace Driver
{

class SH1106
{
public:
    enum class ChargePumpVoltage
    {
        _6_4V,
        _7_4V,
        _8_0V,
        _9_0V
    };

    static constexpr uint8_t Columns = 128;
    static constexpr uint8_t Lines = 8;

    static void init();

    static void fill(uint8_t pattern);

    static void sendCommand(uint8_t code);
    static void sendCommand(uint8_t code, uint8_t arg);

    static void sendData(uint8_t data);

    static void setPowerOn(bool on);
    static void setChargePumpVoltage(ChargePumpVoltage voltage);
    static void setComPadsAltHwConfig(bool alternative);
    static void setComScanInverted(bool inverted);
    static void setContrast(uint8_t value);
    static void setDcDcConvOn(bool on);
    static void setDischargePrechargePeriod(uint8_t precharge, uint8_t discharge);
    static void setDisplayClockDivRatio(uint8_t div, uint8_t ratio);
    static void setDisplayStartLine(uint8_t value);
    static void setDisplayOffset(uint8_t value);
    static void setInvertedDisplay(bool inverted);
    static void setMultiplexRatio(uint8_t value);
    static void setSegmentRemap(bool reverse);
    static void setVcomDeselectLevel(uint8_t value);

    static void setColumn(uint8_t column);
    static void setLine(uint8_t line);
};

}
