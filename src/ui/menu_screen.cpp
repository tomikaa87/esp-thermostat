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
    Created on 2017-01-08
*/

#include "menu_screen.h"
#include "keypad.h"
#include "draw_helper.h"
#include "HeatingController.h"
#include "settings.h"
#include "extras.h"
#include "graphics.h"
#include "text_input.h"
#include "wifi_screen.h"

#include "display/Display.h"
#include "display/Text.h"

#include <stdio.h>

static struct menu_s {
    enum Page{
        PAGE_FIRST = 0,

        PAGE_HEATCTL_MODE = PAGE_FIRST,
        PAGE_DAYTIME_TEMP,
        PAGE_NIGHTTIME_TEMP,
        PAGE_TEMP_OVERSHOOT,
        PAGE_TEMP_UNDERSHOOT,
        PAGE_BOOST_INTVAL,
        PAGE_CUSTOM_TEMP_TIMEOUT,
        PAGE_DISPLAY_BRIGHTNESS,
        PAGE_DISPLAY_TIMEOUT,
        PAGE_TEMP_CORRECTION,
        PAGE_WIFI,

        PAGE_LAST,

        // These pages cannot be accessed by normal navigation
        PAGE_WIFI_PASSWORD
    } page;

    char wifi_psw[64];

    struct persistent_settings new_settings;
} menu;

static void draw_page_heatctl_mode();
static void draw_page_daytime_temp();
static void draw_page_nighttime_temp();
static void draw_page_temp_overshoot();
static void draw_page_temp_undershoot();
static void draw_page_boost_intval();
static void draw_page_custom_temp_timeout();
static void draw_page_display_brightness();
static void draw_page_display_timeout();
static void draw_page_temp_correction();
static void draw_page_wifi();
static void draw_page_wifi_password();
static void update_page_heatctl_mode();
static void update_page_daytime_temp();
static void update_page_nighttime_temp();
static void update_page_temp_overshoot();
static void update_page_temp_undershoot();
static void update_page_boost_intval();
static void update_page_custom_temp_timeout();
static void update_page_temp_correction();
static void update_page_display_brightness();
static void update_page_display_timeout();
static void update_page_wifi();
static void draw_page_title(const char* text);
static void next_page();
static void previous_page();
static void apply_settings();
static void revert_settings();
static void adjust_value(int8_t amount);

void menu_screen_init()
{
    menu.page = menu_s::PAGE_FIRST;
    revert_settings();
}

void menu_screen_draw()
{
    Display::clear();

    switch (menu.page) {
    case menu_s::PAGE_HEATCTL_MODE:
        draw_page_heatctl_mode();
        break;

    case menu_s::PAGE_DAYTIME_TEMP:
        draw_page_daytime_temp();
        break;

    case menu_s::PAGE_NIGHTTIME_TEMP:
        draw_page_nighttime_temp();
        break;

    case menu_s::PAGE_TEMP_OVERSHOOT:
        draw_page_temp_overshoot();
        break;

    case menu_s::PAGE_TEMP_UNDERSHOOT:
        draw_page_temp_undershoot();
        break;

    case menu_s::PAGE_BOOST_INTVAL:
        draw_page_boost_intval();
        break;

    case menu_s::PAGE_CUSTOM_TEMP_TIMEOUT:
        draw_page_custom_temp_timeout();
        break;

    case menu_s::PAGE_DISPLAY_BRIGHTNESS:
        draw_page_display_brightness();
        break;

    case menu_s::PAGE_DISPLAY_TIMEOUT:
        draw_page_display_timeout();
        break;

    case menu_s::PAGE_TEMP_CORRECTION:
        draw_page_temp_correction();
        break;

    case menu_s::PAGE_WIFI:
        draw_page_wifi();
        break;

    case menu_s::PAGE_WIFI_PASSWORD:
        draw_page_wifi_password();
        break;

    case menu_s::PAGE_LAST:
        break;
    }

    if (menu.page != menu_s::PAGE_WIFI) {
        // "<-" previous page indicator
        if (menu.page > menu_s::PAGE_FIRST) {
            Text::draw("<-", 7, 0, 0, false);
        }

        // "->" next page indicator
        if (menu.page < menu_s::PAGE_LAST - 1) {
            Text::draw("->", 7, 115, 0, false);
        }
    }
}

UiResult menu_screen_handle_handle_keys(Keypad::Keys keys)
{
    if (menu.page != menu_s::PAGE_WIFI_PASSWORD && menu.page != menu_s::PAGE_WIFI) {
        // 1: increment current value
        // 2: decrement current value
        // 3: save and exit
        // 4: cancel (revert settings)
        // 5: navigate to previous page
        // 6: navigate to next page

        if (keys & Keypad::Keys::Plus) {
            adjust_value(1);
        } else if (keys & Keypad::Keys::Minus) {
            adjust_value(-1);
        } else if (keys & Keypad::Keys::Menu) {
            apply_settings();
            return UiResult::SwitchMainScreen;
        } else if (keys & Keypad::Keys::Boost) {
            revert_settings();
            return UiResult::SwitchMainScreen;
        } else if (keys & Keypad::Keys::Left) {
            previous_page();
        } else if (keys & Keypad::Keys::Right) {
            next_page();
        }
    } else if (menu.page == menu_s::PAGE_WIFI_PASSWORD) {
        ti_key_event_t key_event;
        bool valid_key = true;

        if (keys & Keypad::Keys::Plus) {
            key_event = TI_KE_UP;
        } else if (keys & Keypad::Keys::Minus) {
            key_event = TI_KE_DOWN;
        } else if (keys & Keypad::Keys::Menu) {
            key_event = TI_KE_SELECT;
        } else if (keys & Keypad::Keys::Left) {
            key_event = TI_KE_LEFT;
        } else if (keys & Keypad::Keys::Right) {
            key_event = TI_KE_RIGHT;
        } else {
            valid_key = false;
        }

        if (valid_key) {
            const ti_key_event_result_t res = text_input_key_event(key_event);

            if (res != TI_KE_NO_ACTION) {
                menu.page = menu_s::PAGE_WIFI;
                menu_screen_draw();
            }
        }
    } else if (menu.page == menu_s::PAGE_WIFI) {
        wifi_screen_key_event(keys);
    }

    return UiResult::Idle;
}

static void draw_page_heatctl_mode()
{
    draw_page_title("MODE");
    update_page_heatctl_mode();
}

static void draw_page_daytime_temp()
{
    draw_page_title("DAYTIME T.");
    update_page_daytime_temp();
}

static void draw_page_nighttime_temp()
{
    draw_page_title("NIGHTTIME T.");
    update_page_nighttime_temp();
}

static void draw_page_temp_overshoot()
{
    draw_page_title("T. OVERSHOOT");
    update_page_temp_overshoot();
}

static void draw_page_temp_undershoot()
{
    draw_page_title("T. UNDERSHOOT");
    update_page_temp_undershoot();
}

static void draw_page_boost_intval()
{
    draw_page_title("BOOST INT. (MIN)");
    update_page_boost_intval();
}

static void draw_page_custom_temp_timeout()
{
    draw_page_title("CUS. TMP. TOUT. (MIN)");
    update_page_custom_temp_timeout();
}

static void draw_page_display_brightness()
{
    draw_page_title("DISP. BRIGHT.");
    update_page_display_brightness();
}

static void draw_page_display_timeout()
{
    draw_page_title("DISP. TIMEOUT");
    update_page_display_timeout();
}

static void draw_page_temp_correction()
{
    draw_page_title("TEMP. CORR.");
    update_page_temp_correction();
}

static void draw_page_wifi()
{
    wifi_screen_init();
}

static void draw_page_wifi_password()
{
    text_input_init(menu.wifi_psw, sizeof(menu.wifi_psw), "WiFi password:");
}

static void update_page_heatctl_mode()
{
    // FIXME: proper mode name must be shown

    char num[3] = { 0 };
    sprintf(num, "%2d", menu.new_settings.heatctl.mode);

    Display::fillArea(0, 3, 128, 3, 0);

    switch (static_cast<HeatingController::Mode>(menu.new_settings.heatctl.mode)) {
    case HeatingController::Mode::Normal:
        graphics_draw_multipage_bitmap(graphics_calendar_icon_20x3p, 20, 3, 20, 2);
        Text::draw("NORMAL", 3, 50, 0, false);
        Text::draw("(SCHEDULE)", 4, 50, 0, false);
        break;

    case HeatingController::Mode::Off:
        graphics_draw_multipage_bitmap(graphics_off_icon_20x3p, 20, 3, 20, 2);
        Text::draw("OFF", 3, 50, 0, false);
        break;
    }
}

static void update_page_daytime_temp()
{
    draw_temperature_value(20,
        menu.new_settings.heatctl.day_temp / 10,
        menu.new_settings.heatctl.day_temp % 10);
}

static void update_page_nighttime_temp()
{
    draw_temperature_value(20,
        menu.new_settings.heatctl.night_temp / 10,
        menu.new_settings.heatctl.night_temp % 10);
}

static void update_page_temp_overshoot()
{
    draw_temperature_value(20,
        menu.new_settings.heatctl.overshoot / 10,
        menu.new_settings.heatctl.overshoot % 10);
}

static void update_page_temp_undershoot()
{
    draw_temperature_value(20,
        menu.new_settings.heatctl.undershoot / 10,
        menu.new_settings.heatctl.undershoot % 10);
}

static void update_page_boost_intval()
{
    char num[3] = { 0 };
    sprintf(num, "%2d", menu.new_settings.heatctl.boost_intval);
    Text::draw7Seg(num, 2, 20);
}

static void update_page_custom_temp_timeout()
{
    char num[5] = { 0 };
    sprintf(num, "%4u", menu.new_settings.heatctl.custom_temp_timeout);
    Text::draw7Seg(num, 2, 20);
}

static void update_page_temp_correction()
{
    draw_temperature_value(20,
        menu.new_settings.heatctl.temp_correction / 10,
        menu.new_settings.heatctl.temp_correction % 10);
}

static void update_page_display_brightness()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", menu.new_settings.display.brightness);
    Text::draw7Seg(num, 2, 20);

    Display::setContrast(menu.new_settings.display.brightness);
}

static void update_page_display_timeout()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", menu.new_settings.display.timeout_secs);
    Text::draw7Seg(num, 2, 20);
}

static void update_page_wifi()
{
    // Text::draw("CONNECTED: YES", 2, 0, 0, false);
    // Text::draw("NETWORK:", 4, 0, 0, false);
    // Text::draw("<SSID>", 5, 0, 0, false);

    wifi_screen_update();
}

static void draw_page_title(const char* text)
{
    Text::draw(text, 0, 0, 0, false);
}

static void next_page()
{
    if (menu.page < menu_s::PAGE_LAST - 1) {
        menu.page = static_cast<menu_s::Page>(static_cast<int>(menu.page) + 1);
        menu_screen_draw();
    }
}

static void previous_page()
{
    if (menu.page > menu_s::PAGE_FIRST) {
        menu.page = static_cast<menu_s::Page>(static_cast<int>(menu.page) - 1);
        menu_screen_draw();
    }
}

static void apply_settings()
{
    settings = menu.new_settings;
    settings_save();
}

static void revert_settings()
{
    menu.new_settings = settings;

    Display::setContrast(settings.display.brightness);
}

static void adjust_value(int8_t amount)
{
    switch (menu.page) {
    case menu_s::PAGE_HEATCTL_MODE:
        if (menu.new_settings.heatctl.mode == 0 && amount > 0) {
            menu.new_settings.heatctl.mode = 2;
        } else if (menu.new_settings.heatctl.mode == 2 && amount < 0) {
            menu.new_settings.heatctl.mode = 0;
        }
        update_page_heatctl_mode();
        break;

    case menu_s::PAGE_DAYTIME_TEMP:
        menu.new_settings.heatctl.day_temp += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.day_temp,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);
        update_page_daytime_temp();
        break;

    case menu_s::PAGE_NIGHTTIME_TEMP:
        menu.new_settings.heatctl.night_temp += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.night_temp,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);
        update_page_nighttime_temp();
        break;

    case menu_s::PAGE_TEMP_OVERSHOOT:
        menu.new_settings.heatctl.overshoot += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.overshoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        update_page_temp_overshoot();
        break;

    case menu_s::PAGE_TEMP_UNDERSHOOT:
        menu.new_settings.heatctl.undershoot += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.undershoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        update_page_temp_undershoot();
        break;

    case menu_s::PAGE_BOOST_INTVAL:
        menu.new_settings.heatctl.boost_intval += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.boost_intval,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX);
        update_page_boost_intval();
        break;

    case menu_s::PAGE_CUSTOM_TEMP_TIMEOUT:
        menu.new_settings.heatctl.custom_temp_timeout += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.custom_temp_timeout,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX);
        update_page_custom_temp_timeout();
        break;

    case menu_s::PAGE_DISPLAY_BRIGHTNESS:
        menu.new_settings.display.brightness += amount;
        update_page_display_brightness();
        break;

    case menu_s::PAGE_DISPLAY_TIMEOUT:
        menu.new_settings.display.timeout_secs += amount;
        update_page_display_timeout();
        break;

    case menu_s::PAGE_TEMP_CORRECTION:
        menu.new_settings.heatctl.temp_correction += amount;
        CLAMP_VALUE(menu.new_settings.heatctl.temp_correction,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX);
        update_page_temp_correction();
        break;

    // TODO dummy implementation
    case menu_s::PAGE_WIFI:
        menu.page = menu_s::PAGE_WIFI_PASSWORD;
        draw_page_wifi_password();
        break;

    case menu_s::PAGE_LAST:
    case menu_s::PAGE_WIFI_PASSWORD:
        break;
    }
}