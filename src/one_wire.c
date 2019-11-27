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

#include "one_wire.h"

#include <Arduino.h>

#define DQ_PIN D7

static inline void ow_dq_float()
{
    pinMode(DQ_PIN, INPUT);
}

static inline void ow_dq_low()
{
    digitalWrite(DQ_PIN, LOW);
    pinMode(DQ_PIN, OUTPUT);
}

static inline void ow_dq_high()
{
    digitalWrite(DQ_PIN, HIGH);
    pinMode(DQ_PIN, OUTPUT);
}

static inline uint8_t ow_dq_read()
{
    return digitalRead(DQ_PIN);
}

uint8_t one_wire_reset()
{
	uint8_t presence, temp;

	ow_dq_float();
	ow_dq_low();
	delayMicroseconds(600);
	ow_dq_float();
	delayMicroseconds(80);
	presence = ow_dq_read();
	delayMicroseconds(600);
	temp = ow_dq_read();

	return !temp ? 2 : presence;
}

void one_wire_write_bit(uint8_t b)
{
	ow_dq_float();
	ow_dq_low();
	delayMicroseconds(5);
	if (b)
		ow_dq_float();
	delayMicroseconds(60);
	ow_dq_high();
	delayMicroseconds(5);
}

void one_wire_write_byte(uint8_t b)
{
	for (uint8_t i = 0; i < 8; ++i) {
		one_wire_write_bit(b & 0x01);
		b >>= 1;
	}
}

uint8_t one_wire_read_bit()
{
	uint8_t data;

	ow_dq_float();
	ow_dq_low();
	delayMicroseconds(10);
	ow_dq_float();
	delayMicroseconds(10);
	data = ow_dq_read();
	delayMicroseconds(40);

	return data;
}

uint8_t one_wire_read_byte()
{
	uint8_t data = 0;

	for (uint8_t i = 0; i < 8; i++) {
		if (one_wire_read_bit())
			data |= (0x01 << i);
	}

	return data;
}

