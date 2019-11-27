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

#ifndef SH1106_H
#define SH1106_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SH1106_I2C_ADDRESS = 0x3Cu,
    SH1106_I2C_DC_FLAG = 0x40u,
    SH1106_I2C_CO_FLAG = 0x80u
};

enum {
    SH1106_LCDWIDTH = 128u,
    SH1106_LCDHEIGHT = 64u,
    SH1106_LCDOFFSET = 2u,
    SH1106_PAGE_COUNT = 8u
};

typedef enum {
    SH1106_CMD_SET_COL_ADDR_LOW             = 0x00u,
    SH1106_CMD_SET_COL_ADDR_HIGH            = 0x10u,
    SH1106_CMD_SET_CHARGE_PUMP_VOLTAGE      = 0x30u,
    SH1106_CMD_SET_DISPLAY_START_LINE       = 0x40u,
    SH1106_CMD_SET_CONTRAST_CONTROL         = 0x81u,
    SH1106_CMD_SET_SEGMENT_REMAP            = 0xA0u,
    SH1106_CMD_SET_ENTIRE_DISPLAY_OFF_ON    = 0xA4u,
    SH1106_CMD_SET_NORMAL_REVERSE_DISPLAY   = 0xA6u,
    SH1106_CMD_SET_MULTIPLEX_RATIO          = 0xA8u,
    SH1106_CMD_SET_DC_DC_OFF_ON             = 0xADu,
    SH1106_CMD_SET_DISPLAY_OFF_ON           = 0xAEu,
    SH1106_CMD_SET_PAGE_ADDR                = 0xB0u,
    SH1106_CMD_SET_COM_OUT_SCAN_DIR         = 0xC0u,
    SH1106_CMD_SET_DISPLAY_OFFSET           = 0xD3u,
    SH1106_CMD_SET_DISPLAY_CLOCK_DIV_RATIO  = 0xD5u,
    SH1106_CMD_SET_PRECHARGE_PERIOD         = 0xD9u,
    SH1106_CMD_SET_COM_PADS_HW_CONFIG       = 0xDAu,
    SH1106_CMD_SET_VCOM_DESELECT_LEVEL      = 0xDBu,
    SH1106_CMD_READ_MODIFY_WRITE            = 0xE0u,
    SH1106_CMD_NOP                          = 0xE3u,
    SH1106_CMD_END_READ_MODIFY_WRITE        = 0xEEu
} sh1106_cmd_t;

typedef enum {
    SH1106_CHARGE_PUMP_6_4V,
    SH1106_CHARGE_PUMP_7_4V,
    SH1106_CHARGE_PUMP_8_0V,
    SH1106_CHARGE_PUMP_9_0V
} sh1106_charge_pump_v_t;

typedef enum {
    SH1106_DIM_BRIGHT,
    SH1106_DIM_NORMAL,
    SH1106_DIM_DARK,
    SH1106_DIM_DARKEST,
} sh1106_dim_level_t;

typedef enum {
    SH1106_PIXEL_COLOR_BLACK,
    SH1106_PIXEL_COLOR_WHITE,
    SH1106_PIXEL_COLOR_INVERT
} sh1106_pixel_color_t;

typedef void (*sh1106_i2c_write_reg_func_t)(const uint8_t reg, const uint8_t data, const void* arg);
typedef void (*sh1106_i2c_write_data_func_t)(const uint8_t* data, const uint8_t length, const void* arg);

void sh1106_set_i2c_write_reg_func(sh1106_i2c_write_reg_func_t func, const void *arg);
void sh1106_set_i2c_write_data_func(sh1106_i2c_write_data_func_t func, const void *arg);

void sh1106_init();

void sh1106_send_command(uint8_t command);
void sh1106_send_data(uint8_t data);

void sh1106_clear();

void sh1106_set_col_addr(uint8_t addr);
void sh1106_set_display_on(bool on);
void sh1106_set_charge_pump_voltage(sh1106_charge_pump_v_t voltage);
void sh1106_set_display_start_line(uint8_t line);
void sh1106_set_contrast(uint8_t level);
void sh1106_set_segment_remap(bool reverse);
void sh1106_set_entire_display_off(bool off);
void sh1106_set_inverted_display(bool inverted);
void sh1106_set_multiplex_ratio(uint8_t ratio);
void sh1106_set_dc_dc_on(bool on);
void sh1106_set_page_addr(uint8_t addr);
void sh1106_set_com_scan_inverted(bool inverted);
void sh1106_set_display_offset(uint8_t offset);
void sh1106_set_display_clock_div_ratio(uint8_t div, uint8_t ratio);
void sh1106_set_discharge_precharge_period(uint8_t precharge, uint8_t discharge);
void sh1106_set_com_pads_hw_config(bool alternative);
void sh1106_set_vcom_deselect_leve(uint8_t level);
void sh1106_start_read_modify_write();
void sh1106_end_read_modify_write();

bool sh1106_is_display_on();

void sh1106_fill_area(
    const uint8_t x, 
    const uint8_t start_page, 
    const uint8_t width, 
    const uint8_t pages, 
    const uint8_t color
);

void sh1106_send_data_array(const uint8_t* data, uint8_t length, uint8_t bit_shift);

#ifdef __cplusplus
}
#endif

#endif // SH1106_H
