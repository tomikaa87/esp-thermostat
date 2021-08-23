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
    Created on 2020-01-08
*/

#pragma once

#if 0

#include "Keypad.h"

#include <stdint.h>

typedef int8_t (* wifi_scan_cb)();
typedef void (* wifi_read_ssid_cb)(int8_t index, char* ssid);
typedef bool (* wifi_is_open_cb)(int8_t index);

void wifi_screen_set_scan_cb(wifi_scan_cb cb);
void wifi_screen_set_read_ssid_cb(wifi_read_ssid_cb cb);
void wifi_screen_set_is_open_cb(wifi_is_open_cb cb);

void wifi_screen_init();
void wifi_screen_update();
void wifi_screen_leave();
void wifi_screen_key_event(Keypad::Keys keys);

#endif