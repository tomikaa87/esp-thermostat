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
    Created on 2017-01-26
*/

#include "Extras.h"

#include <cstring>

uint8_t calculate_schedule_intval_idx(const uint8_t hours, const uint8_t minutes)
{
	return (hours << 1) + (minutes >= 30 ? 1 : 0);
}

std::string Extras::pgmToStdString(PGM_P str)
{
    const auto len = strlen_P(str);
    std::string ss(len, 0);
    memcpy_P(&ss[0], str, len);
    return ss;
}