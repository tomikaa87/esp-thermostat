#pragma once

#include <cstdint>

namespace Text
{
    void draw(char c, uint8_t line, uint8_t x, uint8_t yOffset, bool invert);
    uint8_t draw(const char* s, uint8_t line, uint8_t x, uint8_t yOffset, bool invert);
    void draw7Seg(const char* number, uint8_t line, uint8_t x);
}