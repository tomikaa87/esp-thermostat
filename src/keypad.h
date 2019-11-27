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

#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_R1_C1                       1
#define KEY_R1_C2                       2
#define KEY_R1_C3                       4
#define KEY_R2_C1                       8
#define KEY_R2_C2                       16
#define KEY_R2_C3                       32
#define KEY_LONG_PRESS                  0x8000
#define KEY_CODE_MASK                   0x7FFF

enum {
	KEY_1 = KEY_R1_C1,
	KEY_2 = KEY_R1_C2,
	KEY_3 = KEY_R1_C3,
	KEY_4 = KEY_R2_C1,
	KEY_5 = KEY_R2_C2,
	KEY_6 = KEY_R2_C3
};

void keypad_init();
uint16_t keypad_task();

#ifdef __cplusplus
}
#endif

#endif // KEYPAD_H