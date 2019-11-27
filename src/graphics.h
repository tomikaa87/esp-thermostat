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

#ifndef GRAPHICS_H
#define	GRAPHICS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

extern const uint8_t graphics_flame_icon_20x3p[];
extern const uint8_t graphics_off_icon_20x3p[];
extern const uint8_t graphics_calendar_icon_20x3p[];

void graphics_draw_bitmap(
	const uint8_t* bitmap,
	uint8_t width,
	uint8_t x,
	uint8_t page);

void graphics_draw_multipage_bitmap(
	const uint8_t* mp_bitmap,
	uint8_t width,
	uint8_t page_count,
	uint8_t x,
	uint8_t start_page);

#ifdef	__cplusplus
}
#endif

#endif	/* GRAPHICS_H */

