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

#include "MenuScreen.h"
#include "Keypad.h"
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

MenuScreen::MenuScreen()
{

}

void MenuScreen::menu_screen_init()
{
    _page = Page::First;
    revert_settings();
}

void MenuScreen::menu_screen_draw()
{
    Display::clear();

    switch (_page) {
    case Page::HeatCtlMode:
        draw_page_heatctl_mode();
        break;

    case Page::DaytimeTemp:
        draw_page_daytime_temp();
        break;

    case Page::NightTimeTemp:
        draw_page_nighttime_temp();
        break;

    case Page::TempOvershoot:
        draw_page_temp_overshoot();
        break;

    case Page::TempUndershoot:
        draw_page_temp_undershoot();
        break;

    case Page::BoostInterval:
        draw_page_boost_intval();
        break;

    case Page::CustomTempTimeout:
        draw_page_custom_temp_timeout();
        break;

    case Page::DisplayBrightness:
        draw_page_display_brightness();
        break;

    case Page::DisplayTimeout:
        draw_page_display_timeout();
        break;

    case Page::TempCorrection:
        draw_page_temp_correction();
        break;

    case Page::WiFi:
        draw_page_wifi();
        break;

    case Page::WIFI_PASSWORD:
        draw_page_wifi_password();
        break;

    case Page::Last:
        break;
    }

    if (_page != Page::WiFi) {
        // "<-" previous page indicator
        if (_page > Page::First) {
            Text::draw("<-", 7, 0, 0, false);
        }

        // "->" next page indicator
        if (static_cast<int>(_page) < static_cast<int>(Page::Last) - 1) {
            Text::draw("->", 7, 115, 0, false);
        }
    }
}

UiResult MenuScreen::menu_screen_handle_handle_keys(Keypad::Keys keys)
{
    if (_page != Page::WIFI_PASSWORD && _page != Page::WiFi) {
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
    } else if (_page == Page::WIFI_PASSWORD) {
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
                _page = Page::WiFi;
                menu_screen_draw();
            }
        }
    } else if (_page == Page::WiFi) {
        wifi_screen_key_event(keys);
    }

    return UiResult::Idle;
}

void MenuScreen::draw_page_heatctl_mode()
{
    draw_page_title("MODE");
    update_page_heatctl_mode();
}

void MenuScreen::draw_page_daytime_temp()
{
    draw_page_title("DAYTIME T.");
    update_page_daytime_temp();
}

void MenuScreen::draw_page_nighttime_temp()
{
    draw_page_title("NIGHTTIME T.");
    update_page_nighttime_temp();
}

void MenuScreen::draw_page_temp_overshoot()
{
    draw_page_title("T. OVERSHOOT");
    update_page_temp_overshoot();
}

void MenuScreen::draw_page_temp_undershoot()
{
    draw_page_title("T. UNDERSHOOT");
    update_page_temp_undershoot();
}

void MenuScreen::draw_page_boost_intval()
{
    draw_page_title("BOOST INT. (MIN)");
    update_page_boost_intval();
}

void MenuScreen::draw_page_custom_temp_timeout()
{
    draw_page_title("CUS. TMP. TOUT. (MIN)");
    update_page_custom_temp_timeout();
}

void MenuScreen::draw_page_display_brightness()
{
    draw_page_title("DISP. BRIGHT.");
    update_page_display_brightness();
}

void MenuScreen::draw_page_display_timeout()
{
    draw_page_title("DISP. TIMEOUT");
    update_page_display_timeout();
}

void MenuScreen::draw_page_temp_correction()
{
    draw_page_title("TEMP. CORR.");
    update_page_temp_correction();
}

void MenuScreen::draw_page_wifi()
{
    wifi_screen_init();
}

void MenuScreen::draw_page_wifi_password()
{
    text_input_init(_wifiPsw, sizeof(_wifiPsw), "WiFi password:");
}

void MenuScreen::update_page_heatctl_mode()
{
    // FIXME: proper mode name must be shown

    char num[3] = { 0 };
    sprintf(num, "%2d", _newSettings.heatctl.mode);

    Display::fillArea(0, 3, 128, 3, 0);

    switch (static_cast<HeatingController::Mode>(_newSettings.heatctl.mode)) {
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

void MenuScreen::update_page_daytime_temp()
{
    draw_temperature_value(20,
        _newSettings.heatctl.day_temp / 10,
        _newSettings.heatctl.day_temp % 10);
}

void MenuScreen::update_page_nighttime_temp()
{
    draw_temperature_value(20,
        _newSettings.heatctl.night_temp / 10,
        _newSettings.heatctl.night_temp % 10);
}

void MenuScreen::update_page_temp_overshoot()
{
    draw_temperature_value(20,
        _newSettings.heatctl.overshoot / 10,
        _newSettings.heatctl.overshoot % 10);
}

void MenuScreen::update_page_temp_undershoot()
{
    draw_temperature_value(20,
        _newSettings.heatctl.undershoot / 10,
        _newSettings.heatctl.undershoot % 10);
}

void MenuScreen::update_page_boost_intval()
{
    char num[3] = { 0 };
    sprintf(num, "%2d", _newSettings.heatctl.boost_intval);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::update_page_custom_temp_timeout()
{
    char num[5] = { 0 };
    sprintf(num, "%4u", _newSettings.heatctl.custom_temp_timeout);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::update_page_temp_correction()
{
    draw_temperature_value(20,
        _newSettings.heatctl.temp_correction / 10,
        _newSettings.heatctl.temp_correction % 10);
}

void MenuScreen::update_page_display_brightness()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.display.brightness);
    Text::draw7Seg(num, 2, 20);

    Display::setContrast(_newSettings.display.brightness);
}

void MenuScreen::update_page_display_timeout()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.display.timeout_secs);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::update_page_wifi()
{
    // Text::draw("CONNECTED: YES", 2, 0, 0, false);
    // Text::draw("NETWORK:", 4, 0, 0, false);
    // Text::draw("<SSID>", 5, 0, 0, false);

    wifi_screen_update();
}

void MenuScreen::draw_page_title(const char* text)
{
    Text::draw(text, 0, 0, 0, false);
}

void MenuScreen::next_page()
{
    if (static_cast<int>(_page) < static_cast<int>(Page::Last) - 1) {
        _page = static_cast<Page>(static_cast<int>(_page) + 1);
        menu_screen_draw();
    }
}

void MenuScreen::previous_page()
{
    if (_page > Page::First) {
        _page = static_cast<Page>(static_cast<int>(_page) - 1);
        menu_screen_draw();
    }
}

void MenuScreen::apply_settings()
{
    settings = _newSettings;
    settings_save();
}

void MenuScreen::revert_settings()
{
    _newSettings = settings;

    Display::setContrast(settings.display.brightness);
}

void MenuScreen::adjust_value(int8_t amount)
{
    switch (_page) {
    case Page::HeatCtlMode:
        if (_newSettings.heatctl.mode == 0 && amount > 0) {
            _newSettings.heatctl.mode = 2;
        } else if (_newSettings.heatctl.mode == 2 && amount < 0) {
            _newSettings.heatctl.mode = 0;
        }
        update_page_heatctl_mode();
        break;

    case Page::DaytimeTemp:
        _newSettings.heatctl.day_temp += amount;
        CLAMP_VALUE(_newSettings.heatctl.day_temp,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);
        update_page_daytime_temp();
        break;

    case Page::NightTimeTemp:
        _newSettings.heatctl.night_temp += amount;
        CLAMP_VALUE(_newSettings.heatctl.night_temp,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);
        update_page_nighttime_temp();
        break;

    case Page::TempOvershoot:
        _newSettings.heatctl.overshoot += amount;
        CLAMP_VALUE(_newSettings.heatctl.overshoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        update_page_temp_overshoot();
        break;

    case Page::TempUndershoot:
        _newSettings.heatctl.undershoot += amount;
        CLAMP_VALUE(_newSettings.heatctl.undershoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        update_page_temp_undershoot();
        break;

    case Page::BoostInterval:
        _newSettings.heatctl.boost_intval += amount;
        CLAMP_VALUE(_newSettings.heatctl.boost_intval,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX);
        update_page_boost_intval();
        break;

    case Page::CustomTempTimeout:
        _newSettings.heatctl.custom_temp_timeout += amount;
        CLAMP_VALUE(_newSettings.heatctl.custom_temp_timeout,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX);
        update_page_custom_temp_timeout();
        break;

    case Page::DisplayBrightness:
        _newSettings.display.brightness += amount;
        update_page_display_brightness();
        break;

    case Page::DisplayTimeout:
        _newSettings.display.timeout_secs += amount;
        update_page_display_timeout();
        break;

    case Page::TempCorrection:
        _newSettings.heatctl.temp_correction += amount;
        CLAMP_VALUE(_newSettings.heatctl.temp_correction,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX);
        update_page_temp_correction();
        break;

    // TODO dummy implementation
    case Page::WiFi:
        _page = Page::WIFI_PASSWORD;
        draw_page_wifi_password();
        break;

    case Page::Last:
    case Page::WIFI_PASSWORD:
        break;
    }
}