/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2016-12-30
*/

#pragma once

#include <cstdint>

class Keypad
{
public:
    enum class Keys : uint8_t
    {
        None,

        Row1Col1        = 1 << 0,
        Row1Col2        = 1 << 1,
        Row1Col3        = 1 << 2,

        Row2Col1        = 1 << 3,
        Row2Col2        = 1 << 4,
        Row2Col3        = 1 << 5,

        KeycodeMask     = 0x3f,
        LongPress       = 0x80,

        // Mapping
        Plus            = Row1Col1,
        Minus           = Row1Col2,
        Boost           = Row1Col3,
        Menu            = Row2Col1,
        Right           = Row2Col2,
        Left            = Row2Col3
    };

    Keypad();

    Keys scan();

private:
    enum class State
    {
        Idle,
        Pressed,
        Repeating
    };

    State _state = State::Idle;
    Keys _pressedKeys = Keys::None;
    uint8_t _pressDuration = 0;
    uint8_t _delay = 0;
    uint32_t _lastScanTimestamp = 0;

    Keys readInputs() const;
    void tick();
};

inline Keypad::Keys operator |(const Keypad::Keys k1, const Keypad::Keys k2)
{
    return static_cast<Keypad::Keys>(static_cast<uint8_t>(k1) | static_cast<uint8_t>(k2));
}

inline bool operator &(const Keypad::Keys k1, const Keypad::Keys k2)
{
    return static_cast<Keypad::Keys>(static_cast<uint8_t>(k1) & static_cast<uint8_t>(k2)) != Keypad::Keys::None;
}

inline Keypad::Keys& operator |=(Keypad::Keys& keys, const Keypad::Keys& k)
{
    keys = keys | k;
    return keys;
}
