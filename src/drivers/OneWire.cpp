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
    Created on 2020-01-25
*/

#include "OneWire.h"

using namespace Drivers;

uint8_t Detail::OneWireImpl::reset(const int pin)
{
	noInterrupts();

	busFloat(pin);
	busLow(pin);
	delayMicroseconds(600);
	busFloat(pin);
	delayMicroseconds(80);
	const auto presence = busRead(pin);
	delayMicroseconds(600);
	const auto temp = busRead(pin);

	interrupts();

	return !temp ? 2 : presence;
}

void Detail::OneWireImpl::writeBit(const int pin, const uint8_t b)
{
	noInterrupts();

    busFloat(pin);
	busLow(pin);
	delayMicroseconds(5);
	if (b)
		busFloat(pin);
	delayMicroseconds(60);
	busHigh(pin);
	delayMicroseconds(5);

	interrupts();
}

void Detail::OneWireImpl::writeByte(const int pin, uint8_t b)
{
    for (auto i = 0u; i < sizeof(b) * 8; ++i) {
		writeBit(pin, b & 0x01);
		b >>= 1;
	}
}

uint8_t Detail::OneWireImpl::readBit(const int pin)
{
	noInterrupts();

    busFloat(pin);
    busLow(pin);
	delayMicroseconds(10);
	busFloat(pin);
	delayMicroseconds(10);
	const auto data = busRead(pin);
	delayMicroseconds(40);

	interrupts();

	return data;
}

uint8_t Detail::OneWireImpl::readByte(const int pin)
{
	uint8_t data = 0;

	for (auto i = 0u; i < sizeof(data) * 8; i++) {
		if (readBit(pin))
			data |= (0x01 << i);
	}

	return data;
}

void Detail::OneWireImpl::busLow(const int pin)
{
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);   
}

void Detail::OneWireImpl::busHigh(const int pin)
{
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
}

void Detail::OneWireImpl::busFloat(const int pin)
{
    pinMode(pin, INPUT);
}

uint8_t Detail::OneWireImpl::busRead(const int pin)
{
    return digitalRead(pin);
}