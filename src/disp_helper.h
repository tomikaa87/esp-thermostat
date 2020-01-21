#pragma once

#include <stdbool.h>
#include <stdint.h>

void disp_clear();
void disp_set_page_addressing();
void disp_goto_col(uint8_t col);
void disp_goto_row(uint8_t row);
void disp_send_data(const uint8_t* data, uint8_t length, uint8_t bit_shift, bool invert);