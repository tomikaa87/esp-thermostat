#include "config.h"
#include "disp_helper.h"
#include "sh1106.h"
#include "ssd1306.h"

void disp_clear()
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_clear();
#else
    sh1106_clear();
#endif
}

void disp_set_page_addressing()
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_page_addressing();
#endif
}

void disp_goto_col(uint8_t col)
{
#ifndef CONFIG_USE_OLED_SH1106		
    ssd1306_set_start_column(col);
#else
    sh1106_set_col_addr(col);
#endif
}

void disp_goto_row(uint8_t row)
{
#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_set_page(row);
#else
	sh1106_set_page_addr(row);
#endif
}

void disp_send_data(const uint8_t* data, uint8_t length, uint8_t bit_shift, bool invert)
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_send_data(data, length, bit_shift, invert);
#else
    sh1106_send_data_array(data, length, bit_shift, invert);
#endif    
}