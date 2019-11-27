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
    Created on 2019-10-09
*/

#include "config.h"
#include "Display.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

#include <Arduino.h>
#include <Wire.h>

Display::Display()
{
    Serial.println("Display: initializing Wire");

    Wire.begin();
    Wire.setClock(400000);

#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_set_i2c_send_func([](const uint8_t* const data, const uint8_t length) {
        Wire.beginTransmission(SSD1306_I2C_ADDRESS);
        Wire.write(data, length);
        Wire.endTransmission();
    });

    Serial.println("Display: initializing OLED (SSD1306)");
    ssd1306_init();
#else
    sh1106_set_i2c_write_reg_func([](const uint8_t reg, const uint8_t data, const void*) {
        Wire.beginTransmission(SH1106_I2C_ADDRESS);
        Wire.write(reg);
        Wire.write(&data, 1);
        Wire.endTransmission();
    }, nullptr);

    sh1106_set_i2c_write_data_func([](const uint8_t* data, const uint8_t length, const void*) {
        Wire.beginTransmission(SH1106_I2C_ADDRESS);
        Wire.write(data, length);
        Wire.endTransmission();
    }, nullptr);

    Serial.println("Display: initializing OLED (SH1106)");
    sh1106_init();
#endif
}
