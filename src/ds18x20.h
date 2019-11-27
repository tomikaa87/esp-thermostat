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
    Created on 2017-01-01
*/

#ifndef DS18X20_H
#define	DS18X20_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int16_t ds18x20_last_reading;

void ds18x20_update();

#ifdef	__cplusplus
}
#endif

#endif	/* DS18X20_H */

