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

#ifndef ONE_WIRE_H
#define	ONE_WIRE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>

uint8_t one_wire_reset();

void one_wire_write_bit(uint8_t b);
void one_wire_write_byte(uint8_t b);

uint8_t one_wire_read_bit();
uint8_t one_wire_read_byte();

#ifdef	__cplusplus
}
#endif

#endif	/* ONE_WIRE_H */

