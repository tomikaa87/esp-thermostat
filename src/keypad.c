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

#define SCAN_SAMPLE_COUNT				10
#define SCAN_READ_DELAY					4

#define PRESS_DURATION_UNTIL_REPEAT			15
#define RESET_DELAY_AFTER_KEYS_CHANGED			3
#define REPEAT_INTERVAL					3

#ifndef _DEBUG_SIM_MODE
#define READ_DELAY()	{ delayMicroseconds(SCAN_READ_DELAY); }
#else
#define READ_DELAY()	{}
#endif

#define DRIVE_DELAY()   { delayMicroseconds(7); }

static struct keypad_s {

	enum {
		S_IDLE = 0,
		S_PRESSED,
		S_REPEAT
	} state;

	uint8_t press_duration;
	uint8_t delay_duration;

	uint16_t pressed_keys;
} keypad;

static uint16_t scan();

void keypad_timer_tick()
{
	if (keypad.press_duration < 255)
		++keypad.press_duration;

	if (keypad.delay_duration > 0)
		--keypad.delay_duration;
}

void keypad_init()
{
	memset(&keypad, 0, sizeof(struct keypad_s));

	pinMode(D0, INPUT_PULLUP);
	pinMode(D3, INPUT_PULLUP);
	pinMode(D4, INPUT_PULLUP);
	pinMode(D5, INPUT_PULLUP);
	pinMode(D6, INPUT_PULLUP);
}

uint16_t keypad_task()
{
    static uint32_t lastMillis = 0;

    if (millis() > lastMillis + 15) {
        keypad_timer_tick();
        lastMillis = millis();
    }

	// Avoid using the interrupt which interferes with the 
	// display and DS18x20 drivers
//	if (TMR2_HasOverflowOccured()) {
//		keypad_timer_tick();
//	}
	
	if (keypad.delay_duration > 0)
		return 0;

	uint16_t scanned_keys = scan();

	switch (keypad.state) {
	case S_IDLE:
		// If there is a pressed key, start measuring the
		// duration until we activate long press mode
		if (scanned_keys > 0) {
			keypad.state = S_PRESSED;
			keypad.press_duration = 0;
			keypad.pressed_keys = scanned_keys;
			return keypad.pressed_keys;
		}
		break;

	case S_PRESSED:
		// If there was a change, reset the state machine
		// and wait a little bit
		if (keypad.pressed_keys != scanned_keys) {
			keypad.state = S_IDLE;
			keypad.delay_duration = RESET_DELAY_AFTER_KEYS_CHANGED;
		} else {
			// The keys are pressed long enough to start repeating
			if (keypad.press_duration > PRESS_DURATION_UNTIL_REPEAT) {
				keypad.state = S_REPEAT;
				keypad.delay_duration = REPEAT_INTERVAL;
				return keypad.pressed_keys | KEY_LONG_PRESS;
			}
		}
		break;

	case S_REPEAT:
		// If there was a change, reset the state machine
		// and wait a little bit
		if (keypad.pressed_keys != scanned_keys) {
			keypad.state = S_IDLE;
			keypad.delay_duration = RESET_DELAY_AFTER_KEYS_CHANGED;
		} else {
			// Keep on repeating and resetting the delay counter
			keypad.delay_duration = REPEAT_INTERVAL;
			return keypad.pressed_keys | KEY_LONG_PRESS;
		}
		break;
	}

	return 0;
}

/*** Internal functions *******************************************************/

/**
 * Reads the keypad with software de-bouncing
 */
static uint16_t scan()
{
	uint16_t pressed_keys = 0;

	for (uint8_t samples = 0; samples < SCAN_SAMPLE_COUNT; ++samples) {
		uint16_t code = 0;

		// Row 1
		digitalWrite(D5, LOW);
        pinMode(D5, OUTPUT);

        // Col 1
        READ_DELAY();
        if (digitalRead(D0) == 0) {
            code |= KEY_R1_C1;
        }

        // Col 2
        READ_DELAY();
        if (digitalRead(D3) == 0) {
            code |= KEY_R1_C2;
        }

        // Col 3
        READ_DELAY();
        if (digitalRead(D4) == 0) {
            code |= KEY_R1_C3;
        }

        pinMode(D5, INPUT_PULLUP);
        READ_DELAY();
        DRIVE_DELAY();

		// Row 2
        digitalWrite(D6, LOW);
        pinMode(D6, OUTPUT);

        // Col 1
        READ_DELAY();
        if (digitalRead(D0) == 0) {
            code |= KEY_R2_C1;
        }

        // Col 2
        READ_DELAY();
        if (digitalRead(D3) == 0) {
            code |= KEY_R2_C2;
        }

        // Col 3
        READ_DELAY();
        if (digitalRead(D4) == 0) {
            code |= KEY_R2_C3;
        }

        pinMode(D6, INPUT_PULLUP);
        READ_DELAY();

        pressed_keys |= code;
	}

	return pressed_keys;
}