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

#include "config.h"
#include "disp_helper.h"
#include "keypad.h"
#include "text.h"
#include "text_input.h"
#include "wifi.h"
#include "wifi_screen.h"

#include <string.h>
#include <stdio.h>

/*
    Main screen:

    WiFi Connection
    Connected to:
    <SSID>
    IP: 000.000.000.000

    > Disconnect
      Scan
      Leave


    Scan screen (scanning):

    WiFi Networks:
    
    Scanning...

    > Abort


    Scan screen (found networks):

    WiFi Networks:
    > SSID 1
      SSID 2
      SSID 3
      SSID 4
      SSID 5
      Back
      


*/

typedef enum {
    SCR_MAIN,
    SCR_SCAN,
    SCR_PASSWORD
} screen_t;

typedef enum {
    SCN_IDLE,
    SCN_SCANNING,
    SCN_FINISHED
} scan_state_t;

static struct state_s {
    int ap_cnt;
    char ap_psk[64];
    char ap_ssid[32];
    int list_pos;
    int list_offs;
    screen_t screen;
    scan_state_t scan;
} state;

static struct cb_s {
    wifi_scan_cb scan;
    wifi_read_ssid_cb read_ssid;
    wifi_is_open_cb is_open;
} callbacks;

static void draw();
static void update();
static void select_item();
static void init_scan();

void wifi_screen_set_scan_cb(wifi_scan_cb cb)
{
    callbacks.scan = cb;
}

void wifi_screen_set_read_ssid_cb(wifi_read_ssid_cb cb)
{
    callbacks.read_ssid = cb;
}

void wifi_screen_set_is_open_cb(wifi_is_open_cb cb)
{
    callbacks.is_open = cb;
}

void wifi_screen_init()
{
    printf("wifi_screen::init\n");

    memset(&state, 0, sizeof(struct state_s));

    draw();
}

void wifi_screen_update()
{
    printf("wifi_screen::update\n");
}

void wifi_screen_leave()
{
    printf("wifi_screen::leave\n");
}

void wifi_screen_key_event(uint16_t keys)
{
    printf("wifi_screen::key_event: keys=0x%x\n", keys);

    bool update_needed = false;

    switch (state.screen) {
    case SCR_MAIN:
        // + -> Down
        if (keys & KEY_1) {
            if (++state.list_pos >= 3) {
                state.list_pos = 0;
            }
            update_needed = true;
        // - -> Up
        } else if (keys & KEY_2) {
            if (state.list_pos == 0) {
                state.list_pos = 2;
            } else {
                --state.list_pos;
            }
            update_needed = true;
        // Right -> Select
        } else if (keys & KEY_6) {
            select_item();
        }
        break;

    case SCR_SCAN:
        // + -> Down
        if (keys & KEY_1) {
            if (state.list_pos < state.ap_cnt) {
                ++state.list_pos;
                if (state.list_pos - state.list_offs > 6) {
                    ++state.list_offs;
                }
            }
            update_needed = true;
        // - -> Up
        } else if (keys & KEY_2) {
            if (state.list_pos > 0) {
                --state.list_pos;
                if (state.list_pos - state.list_offs < 0) {
                    --state.list_offs;
                }
            }
            update_needed = true;
        } else if (keys & KEY_6) {
            select_item();
        }

    case SCR_PASSWORD: {
        ti_key_event_t key_event;
		bool valid_key = true;

		if (keys & KEY_1) {
			key_event = TI_KE_UP;
		} else if (keys & KEY_2) {
			key_event = TI_KE_DOWN;
		} else if (keys & KEY_3) {
			key_event = TI_KE_SELECT;
		} else if (keys & KEY_5) {
			key_event = TI_KE_LEFT;
		} else if (keys & KEY_6) {
			key_event = TI_KE_RIGHT;
		} else {
			valid_key = false;
		}

		if (valid_key) {
			const ti_key_event_result_t res = text_input_key_event(key_event);

            switch (res) {
            case TI_KE_ACCEPT:
                printf("wifi_screen: password entered: %s\n", state.ap_psk);
                state.screen = SCR_MAIN;
                draw();
                break;

            case TI_KE_CANCEL:
                printf("wifi_screen: password input canceled\n");
                state.screen = SCR_SCAN;
                draw();
                break;

            default:
                break;
            }
		}
        break;
    }        
    }

    if (update_needed) {
        update();
    }
}

static void draw()
{
    printf("wifi_screen::draw: screen=%d\n", state.screen);

    disp_clear();

    switch (state.screen) {
    case SCR_MAIN:
        text_draw("WiFi Connection", 0, 0, 0, false);
        text_draw("Connected to:", 1, 0, 0, false);
        text_draw("IP:", 3, 0, 0, false);
        text_draw("Disconnect", 5, 10, 0, false);
        text_draw("Scan", 6, 10, 0, false);
        text_draw("Leave", 7, 10, 0, false);
        break;

    case SCR_SCAN:
        text_draw("WiFi Networks", 0, 0, 0, false);
        switch (state.scan) {
        case SCN_IDLE:
            text_draw("Idle", 2, 10, 0, false);
            break;

        case SCN_SCANNING:
            text_draw("Scanning...", 2, 10, 0, false);
            break;
        }
        break;

    case SCR_PASSWORD:
        text_input_init(state.ap_psk, sizeof(state.ap_psk), "WiFi Password:");
        break;
    }

    update();
}

static void update()
{
    printf("wifi_screen::update: screen=%d\n", state.screen);

    switch (state.screen) {
    case SCR_MAIN:
        for (int i = 0; i < 3; ++i) {
            text_draw(i == state.list_pos ? ">" : " ", i + 5, 0, 0, false);
        }
        break;

    case SCR_SCAN:
        switch (state.scan) {
        case SCN_FINISHED:
            for (int i = state.list_offs, line = 1; i < state.ap_cnt + 1 && i - state.list_offs < 7; ++i, ++line) {
                printf("wifi_screen::update: drawing AP list item, i=%d, list_pos=%d, list_offs=%d, line=%d\n",
                    i, state.list_pos, state.list_offs, line
                );

                // Clear the row
                disp_goto_col(10);
                disp_goto_row(line);
                for (int i = 0; i < 118; ++i) {
                    static const uint8_t pattern = 0;
                    disp_send_data(&pattern, 1, 0, false);
                }

                if (i == 0) {
                    text_draw("<< Back", line, 10, 0, false);
                } else {
                    if (callbacks.read_ssid) {
                        char buf[32] = { 0 };
                        callbacks.read_ssid(i - 1, buf);
                        text_draw(buf, line, 10, 0, false);
                    }
                }
                text_draw(i == state.list_pos ? ">" : " ", line, 0, 0, false);
            }
            break;
        }
        break;
    }
}

static void select_item()
{
    printf("wifi_screen::select_item: %d\n", state.list_pos);

    switch (state.screen) {
    case SCR_MAIN:
        if (state.list_pos == 0) {
            // Disconnect
        } else if (state.list_pos == 1) {
            // Scan
            state.screen = SCR_SCAN;
            draw();
            init_scan();
        } else if (state.list_pos == 2) {
            // Leave
        }
        break;

    case SCR_SCAN:
        if (state.list_pos == 0) {
            // Back
            state.screen = SCR_MAIN;
            draw();
        } else {
            // Select SSID
            if (callbacks.read_ssid) {
                callbacks.read_ssid(state.list_pos - 1, state.ap_ssid);
                printf("wifi_screen::select_item: selected SSID: %s\n", state.ap_ssid);
            }

            if (callbacks.is_open) {
                if (!callbacks.is_open(state.list_pos - 1)) {
                    printf("wifi_screen::select_item: selected AP requires password\n");
                    state.screen = SCR_PASSWORD;
                    text_input_init(state.ap_psk, sizeof(state.ap_psk), "Enter password:");
                } else {
                    state.screen = SCR_MAIN;
                    printf("wifi_screen::select_item: selected AP is open\n");
                    draw();
                }
            }
        }
        break;
    }
}

static void init_scan()
{
    printf("wifi_screen::init_scan\n");

    state.scan = SCN_IDLE;
    state.ap_cnt = 0;
    state.list_offs = 0;

    update();

    if (callbacks.scan) {
        state.scan = SCN_SCANNING;
        update();
        printf("wifi_screen::init_scan: scanning\n");
        state.ap_cnt = callbacks.scan();
        printf("wifi_screen::init_scan: finished, count=%d\n", state.ap_cnt);
        state.scan = SCN_FINISHED;
        update();
    } else {
        printf("wifi_screen::init_scan: scan callback is null\n");
    }
}