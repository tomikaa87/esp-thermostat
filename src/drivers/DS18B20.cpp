#include "DS18B20.h"

using namespace Drivers;

int16_t DS18B20::_lastReading = 0;

void DS18B20::update()
{
	static bool convert = true;
	
	if (convert) {
		startConversion();
	} else {
		int16_t t = readSensor();

		// t += settings.heatctl.temp_correction * 10;
		_lastReading = t;
	}
	
	convert = !convert;
}

int16_t DS18B20::lastReading()
{
    return _lastReading;
}

void DS18B20::startConversion()
{
    Bus::reset();
    Bus::writeByte(0xCC);
    Bus::writeByte(0x44);
}

int16_t DS18B20::readSensor()
{
	Bus::reset();
	Bus::writeByte(0xCC);
	Bus::writeByte(0xBE);

	uint8_t lsb = Bus::readByte();
	uint8_t msb = Bus::readByte();
	uint16_t value = (msb << 8) + lsb;

	if (value & 0x8000) {
		value = ~value + 1;
	}
	
	int16_t celsius = (value >> (ResolutionBits - 8)) * 100;
	uint16_t frac_part = (value << (4 - (ResolutionBits - 8))) & 0xf;
	frac_part *= 625;
	celsius += frac_part / 100;
	
	if (value & 0x8000)
		celsius *= -1;

	return celsius;
}