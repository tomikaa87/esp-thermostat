#pragma once

typedef enum {
    TI_KE_UP,
    TI_KE_DOWN,
    TI_KE_LEFT,
    TI_KE_RIGHT,
    TI_KE_SELECT
} ti_key_event_t;

typedef enum {
    TI_KE_NO_ACTION,
    TI_KE_ACCEPT,
    TI_KE_CANCEL
} ti_key_event_result_t;

void text_input_init(char* buf, int maxlen, const char *title);
ti_key_event_result_t text_input_key_event(ti_key_event_t event);
