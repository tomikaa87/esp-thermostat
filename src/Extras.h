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
    Created on 2017-01-02
*/

#ifndef EXTRAS_H
#define	EXTRAS_H

#include <stdint.h>

// Integer division from Linux kernel
/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(n, d) \
    ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

#define CLAMP_VALUE(_VAL, _MIN, _MAX) \
	((_VAL) = (_VAL) >= (_MAX) ? (_MAX) : ((_VAL) <= (_MIN) ? (_MIN) : (_VAL)))

uint8_t calculate_schedule_intval_idx(uint8_t hours, uint8_t minutes);

namespace Extras
{
    template <typename ValueType, typename MinType, typename MaxType>
    ValueType constexpr clampValue(ValueType value, MinType min, MaxType max)
    {
        return value <= min ? min : value >= max ? max : value;
    }

    template <typename ValueType, typename AdjType, typename MinType, typename MaxType>
    ValueType adjustValueWithRollOver(ValueType value, AdjType adjustment, MinType min, MaxType max)
    {
        auto newValue = value + adjustment;

        if (newValue < min || newValue > max) {
            if (adjustment >= 0) {
                // Rolling over to minimum
                return min;
            } else {
                // Rolling over to maximum
                return max;
            }
        }

        return newValue;
    }
}

#endif	/* EXTRAS_H */

