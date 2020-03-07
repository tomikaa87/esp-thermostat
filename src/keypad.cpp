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

#include "keypad.h"

#include <Arduino.h>
#include <string.h>

namespace Params
{
	static constexpr auto ScanSampleCount = 10;
	static constexpr auto ScanReadDelay = 4;
	static constexpr auto ScanDriveDelay = 7;
	
	static constexpr auto PressDurationToRepeat = 15;
	static constexpr auto ResetDelayAfterKeysChanged = 3;
	static constexpr auto RepeatInterval = 3;

	static constexpr auto DriveDelay = 7;
}

Keypad::Keypad()
{
	// TODO move pin numbers to a common config file
	pinMode(D0, INPUT_PULLUP);
	pinMode(D3, INPUT_PULLUP);
	pinMode(D4, INPUT_PULLUP);
	pinMode(D5, INPUT_PULLUP);
	pinMode(D6, INPUT_PULLUP);
}

Keypad::Keys Keypad::scan()
{
    if (millis() > _lastScanTimestamp + 15) {
        tick();
        _lastScanTimestamp = millis();
    }

	if (_delay > 0)
		return Keys::None;

	const auto readKeys = readInputs();

	switch (_state)
	{
		case State::Idle:
			// If there is a pressed key, start measuring the
			// duration until we activate long press mode
			if (readKeys != Keys::None) {
				_state = State::Pressed;
				_pressDuration = 0;
				_pressedKeys = readKeys;
				return _pressedKeys;
			}
			break;

		case State::Pressed:
			// If there was a change, reset the state machine
			// and wait a little bit
			if (_pressedKeys != readKeys) {
				_state = State::Idle;
				_delay = Params::ResetDelayAfterKeysChanged;
			} else {
				// The keys are pressed long enough to start repeating
				if (_pressDuration > Params::PressDurationToRepeat) {
					_state = State::Repeating;
					_delay = Params::RepeatInterval;
					return _pressedKeys | Keys::LongPress;
				}
			}
			break;

		case State::Repeating:
			// If there was a change, reset the state machine
			// and wait a little bit
			if (_pressedKeys != readKeys) {
				_state = State::Idle;
				_delay = Params::ResetDelayAfterKeysChanged;
			} else {
				// Keep on repeating and resetting the delay counter
				_delay = Params::RepeatInterval;
				return _pressedKeys | Keys::LongPress;
			}
			break;
	}

	return Keys::None;
}

/**
 * Reads the keypad with software de-bouncing
 */
Keypad::Keys Keypad::readInputs() const
{
	Keys pressed = Keys::None;

	for (uint8_t samples = 0; samples < Params::ScanSampleCount; ++samples) {
		Keys readKeys = Keys::None;

		// Row 1
		digitalWrite(D5, LOW);
        pinMode(D5, OUTPUT);

        // Col 1
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D0) == 0) {
            readKeys |= Keys::Row1Col1;
        }

        // Col 2
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D3) == 0) {
            readKeys |= Keys::Row1Col2;
        }

        // Col 3
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D4) == 0) {
            readKeys |= Keys::Row1Col3;
        }

        pinMode(D5, INPUT_PULLUP);
        delayMicroseconds(Params::ScanReadDelay);
        delayMicroseconds(Params::ScanDriveDelay);

		// Row 2
        digitalWrite(D6, LOW);
        pinMode(D6, OUTPUT);

        // Col 1
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D0) == 0) {
            readKeys |= Keys::Row2Col1;
        }

        // Col 2
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D3) == 0) {
            readKeys |= Keys::Row2Col2;
        }

        // Col 3
        delayMicroseconds(Params::ScanReadDelay);
        if (digitalRead(D4) == 0) {
            readKeys |= Keys::Row2Col3;
        }

        pinMode(D6, INPUT_PULLUP);
        delayMicroseconds(Params::ScanReadDelay);

        pressed |= readKeys;
	}

	return pressed;
}

void Keypad::tick()
{
	if (_pressDuration < 255)
		++_pressDuration;

	if (_delay > 0)
		--_delay;
}