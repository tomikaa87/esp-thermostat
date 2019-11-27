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
    Created on 2019-11-27
*/

#include "sh1106.h"

#include <stdio.h>

static uint8_t sh1106_display_on = 0;

static sh1106_i2c_write_reg_func_t i2c_write_reg = NULL;
static const void* i2c_write_reg_func_arg = NULL;

static sh1106_i2c_write_data_func_t i2c_write_data = NULL;
static const void* i2c_write_data_func_arg = NULL;

void sh1106_set_i2c_write_reg_func(sh1106_i2c_write_reg_func_t func, const void* arg)
{
    i2c_write_reg = func;
    i2c_write_reg_func_arg = arg;
}

void sh1106_set_i2c_write_data_func(sh1106_i2c_write_data_func_t func, const void *arg)
{
    i2c_write_data = func;
    i2c_write_data_func_arg = arg;
}

void sh1106_init()
{
    sh1106_set_display_on(false);

    sh1106_set_charge_pump_voltage(SH1106_CHARGE_PUMP_8_0V);
    sh1106_set_display_start_line(0);
    sh1106_set_contrast(0x80);
    sh1106_set_segment_remap(true);
    sh1106_set_inverted_display(false);
    sh1106_set_multiplex_ratio(63);
    sh1106_set_dc_dc_on(true);
    sh1106_set_com_scan_inverted(true);
    sh1106_set_display_offset(0);
    sh1106_set_display_clock_div_ratio(0, 0x0F);
    sh1106_set_discharge_precharge_period(2, 2);
    sh1106_set_com_pads_hw_config(true);
    sh1106_set_vcom_deselect_level(0x35u);

    sh1106_clear();
    sh1106_set_display_on(true);
}

void sh1106_send_command(uint8_t command)
{
    if (i2c_write_reg)
        i2c_write_reg(0, command, i2c_write_reg_func_arg);
}

void sh1106_send_data(uint8_t data)
{
    if (i2c_write_reg)
        i2c_write_reg(SH1106_I2C_DC_FLAG, data, i2c_write_reg_func_arg);
}

void sh1106_clear()
{
    sh1106_set_col_addr(0);

    for (uint8_t page = 0; page < SH1106_PAGE_COUNT; ++page)
    {
        sh1106_set_page_addr(page);

        for (uint8_t i = 0; i < 132; ++i)
            sh1106_send_data(0);
    }
}

void sh1106_set_col_addr(uint8_t addr)
{
    addr += SH1106_LCDOFFSET;
    sh1106_send_command(SH1106_CMD_SET_COL_ADDR_LOW | (addr & 0x0Fu));
    sh1106_send_command(SH1106_CMD_SET_COL_ADDR_HIGH | ((addr >> 4) & 0x0Fu));
}

void sh1106_set_display_on(bool on)
{
    sh1106_display_on = on;
    sh1106_send_command(SH1106_CMD_SET_DISPLAY_OFF_ON | (on ? 1u : 0u));
}

void sh1106_set_charge_pump_voltage(sh1106_charge_pump_v_t voltage)
{
    sh1106_send_command(SH1106_CMD_SET_CHARGE_PUMP_VOLTAGE | ((uint8_t)voltage & 0x03u));
}

void sh1106_set_display_start_line(uint8_t line)
{
    sh1106_send_command(SH1106_CMD_SET_DISPLAY_START_LINE | (line & 0x3Fu));
}

void sh1106_set_contrast(uint8_t level)
{
    sh1106_send_command(SH1106_CMD_SET_CONTRAST_CONTROL);
    sh1106_send_command(level);
}

void sh1106_set_segment_remap(bool reverse)
{
    sh1106_send_command(SH1106_CMD_SET_SEGMENT_REMAP | (reverse ? 1u : 0u));
}

void sh1106_set_entire_display_off(bool off)
{
    sh1106_send_command(SH1106_CMD_SET_ENTIRE_DISPLAY_OFF_ON | (off ? 1u : 0u));
}

void sh1106_set_inverted_display(bool inverted)
{
    sh1106_send_command(SH1106_CMD_SET_NORMAL_REVERSE_DISPLAY | (inverted ? 1u : 0u));
}

void sh1106_set_multiplex_ratio(uint8_t ratio)
{
    sh1106_send_command(SH1106_CMD_SET_MULTIPLEX_RATIO);
    sh1106_send_command(ratio & 0x3Fu);
}

void sh1106_set_dc_dc_on(bool on)
{
    sh1106_send_command(SH1106_CMD_SET_DC_DC_OFF_ON);
    sh1106_send_command(0x8Au | (on ? 1u : 0u));
}

void sh1106_set_page_addr(uint8_t addr)
{
    sh1106_send_command(SH1106_CMD_SET_PAGE_ADDR | (addr & 0x0F));
}

void sh1106_set_com_scan_inverted(bool inverted)
{
    sh1106_send_command(SH1106_CMD_SET_COM_OUT_SCAN_DIR | (inverted ? 0x08u : 0u));
}

void sh1106_set_display_offset(uint8_t offset)
{
    sh1106_send_command(SH1106_CMD_SET_DISPLAY_OFFSET);
    sh1106_send_command(offset & 0x3Fu);
}

void sh1106_set_display_clock_div_ratio(uint8_t div, uint8_t ratio)
{
    sh1106_send_command(SH1106_CMD_SET_DISPLAY_CLOCK_DIV_RATIO);
    sh1106_send_command((div & 0x0Fu) | ((uint8_t)(ratio << 4) & 0xF0u));
}

void sh1106_set_discharge_precharge_period(uint8_t precharge, uint8_t discharge)
{
    sh1106_send_command(SH1106_CMD_SET_PRECHARGE_PERIOD);
    sh1106_send_command((precharge & 0x0Fu) | ((uint8_t)(discharge << 4) & 0x0Fu));
}

void sh1106_set_com_pads_hw_config(bool alternative)
{
    sh1106_send_command(SH1106_CMD_SET_COM_PADS_HW_CONFIG);
    sh1106_send_command(alternative ? 0x12u : 0x02u);
}

void sh1106_set_vcom_deselect_level(uint8_t level)
{
    sh1106_send_command(SH1106_CMD_SET_VCOM_DESELECT_LEVEL);
    sh1106_send_command(level);
}

void sh1106_start_read_modify_write()
{
    sh1106_send_command(SH1106_CMD_READ_MODIFY_WRITE);
}

void sh1106_end_read_modify_write()
{
    sh1106_send_command(SH1106_CMD_END_READ_MODIFY_WRITE);
}

bool sh1106_is_display_on()
{
    return sh1106_display_on;
}

void sh1106_fill_area(
    const uint8_t x, 
    const uint8_t start_page, 
    const uint8_t width, 
    const uint8_t pages, 
    const uint8_t color
)
{
	if (width == 0 || x >= SH1106_LCDWIDTH)
		return;
	
	for (uint8_t i = 0; i < pages; ++i) {
		uint8_t page = start_page + i;
		if (page >= 8)
			return;
		
		sh1106_set_page_addr(page);
		sh1106_set_col_addr(x);
		
		for (uint8_t j = 0; j < width && x + j < SH1106_LCDWIDTH; ++j) {
            sh1106_send_data(color ? 0xFFu : 0u);
		}
	}
}

void sh1106_send_data_array(const uint8_t* data, uint8_t length, uint8_t bit_shift)
{
    if (bit_shift > 7)
		return;
	
    uint8_t buffer[17];
	uint8_t bytes_remaining = length;
	uint8_t data_index = 0;
	static const uint8_t chunk_size = 16;

	while (bytes_remaining > 0) {
		uint8_t count = bytes_remaining >= chunk_size ? chunk_size : bytes_remaining;
		bytes_remaining -= count;

		buffer[0] = SH1106_I2C_DC_FLAG;
		for (uint8_t i = 1; i <= count; ++i)
			buffer[i] = data[data_index++] << bit_shift;
		
		i2c_write_data(buffer, count + 1, i2c_write_data_func_arg);
	}
}