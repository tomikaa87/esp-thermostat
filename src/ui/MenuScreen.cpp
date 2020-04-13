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
    : Screen("Menu")
{
}

void MenuScreen::activate()
{
    _page = Page::First;
    revertSettings();
    draw();
}

void MenuScreen::update()
{
}

Screen::Action MenuScreen::keyPress(Keypad::Keys keys)
{
    if (_page != Page::WIFI_PASSWORD && _page != Page::WiFi) {
        // 1: increment current value
        // 2: decrement current value
        // 3: save and exit
        // 4: cancel (revert settings)
        // 5: navigate to previous page
        // 6: navigate to next page

        if (keys & Keypad::Keys::Plus) {
            adjustValue(1);
        } else if (keys & Keypad::Keys::Minus) {
            adjustValue(-1);
        } else if (keys & Keypad::Keys::Menu) {
            applySettings();
            return Action::NavigateBack;
        } else if (keys & Keypad::Keys::Boost) {
            revertSettings();
            return Action::NavigateBack;
        } else if (keys & Keypad::Keys::Left) {
            previousPage();
        } else if (keys & Keypad::Keys::Right) {
            nextPage();
        }
    } else if (_page == Page::WIFI_PASSWORD) {
        ti_key_event_t keyEvent;
        bool validKey = true;

        if (keys & Keypad::Keys::Plus) {
            keyEvent = TI_KE_UP;
        } else if (keys & Keypad::Keys::Minus) {
            keyEvent = TI_KE_DOWN;
        } else if (keys & Keypad::Keys::Menu) {
            keyEvent = TI_KE_SELECT;
        } else if (keys & Keypad::Keys::Left) {
            keyEvent = TI_KE_LEFT;
        } else if (keys & Keypad::Keys::Right) {
            keyEvent = TI_KE_RIGHT;
        } else {
            validKey = false;
        }

        if (validKey) {
            const ti_key_event_result_t res = text_input_key_event(keyEvent);

            if (res != TI_KE_NO_ACTION) {
                _page = Page::WiFi;
                draw();
            }
        }
    } else if (_page == Page::WiFi) {
        wifi_screen_key_event(keys);
    }

    return Action::NoAction;
}

void MenuScreen::draw()
{
    Display::clear();

    switch (_page) {
    case Page::HeatCtlMode:
        drawPageHeatCtlMode();
        break;

    case Page::DaytimeTemp:
        drawPageDaytimeTemp();
        break;

    case Page::NightTimeTemp:
        drawPageNightTimeTemp();
        break;

    case Page::TempOvershoot:
        drawPageTempOvershoot();
        break;

    case Page::TempUndershoot:
        drawPageTempUndershoot();
        break;

    case Page::BoostInterval:
        drawPageBoostIntval();
        break;

    case Page::CustomTempTimeout:
        drawPageCustomTempTimeout();
        break;

    case Page::DisplayBrightness:
        drawPageDisplayBrightness();
        break;

    case Page::DisplayTimeout:
        drawPageDisplayTimeout();
        break;

    case Page::TempCorrection:
        drawPageTempCorrection();
        break;

    case Page::WiFi:
        drawPageWifi();
        break;

    case Page::WIFI_PASSWORD:
        drawPageWifiPassword();
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

void MenuScreen::drawPageHeatCtlMode()
{
    drawPageTitle("MODE");
    updatePageHeatCtlMode();
}

void MenuScreen::drawPageDaytimeTemp()
{
    drawPageTitle("DAYTIME T.");
    updatePageDaytimeTemp();
}

void MenuScreen::drawPageNightTimeTemp()
{
    drawPageTitle("NIGHTTIME T.");
    updatePageNightTimeTemp();
}

void MenuScreen::drawPageTempOvershoot()
{
    drawPageTitle("T. OVERSHOOT");
    updatePageTempOvershoot();
}

void MenuScreen::drawPageTempUndershoot()
{
    drawPageTitle("T. UNDERSHOOT");
    updatePageTempUndershoot();
}

void MenuScreen::drawPageBoostIntval()
{
    drawPageTitle("BOOST INT. (MIN)");
    updatePageBoostIntval();
}

void MenuScreen::drawPageCustomTempTimeout()
{
    drawPageTitle("CUS. TMP. TOUT. (MIN)");
    updatePageCustomTempTimeout();
}

void MenuScreen::drawPageDisplayBrightness()
{
    drawPageTitle("DISP. BRIGHT.");
    updatePageDisplayBrightness();
}

void MenuScreen::drawPageDisplayTimeout()
{
    drawPageTitle("DISP. TIMEOUT");
    updatePageDisplayTimeout();
}

void MenuScreen::drawPageTempCorrection()
{
    drawPageTitle("TEMP. CORR.");
    updatePageTempCorrection();
}

void MenuScreen::drawPageWifi()
{
    wifi_screen_init();
}

void MenuScreen::drawPageWifiPassword()
{
    text_input_init(_wifiPsw, sizeof(_wifiPsw), "WiFi password:");
}

void MenuScreen::updatePageHeatCtlMode()
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

void MenuScreen::updatePageDaytimeTemp()
{
    draw_temperature_value(20,
        _newSettings.heatctl.day_temp / 10,
        _newSettings.heatctl.day_temp % 10);
}

void MenuScreen::updatePageNightTimeTemp()
{
    draw_temperature_value(20,
        _newSettings.heatctl.night_temp / 10,
        _newSettings.heatctl.night_temp % 10);
}

void MenuScreen::updatePageTempOvershoot()
{
    draw_temperature_value(20,
        _newSettings.heatctl.overshoot / 10,
        _newSettings.heatctl.overshoot % 10);
}

void MenuScreen::updatePageTempUndershoot()
{
    draw_temperature_value(20,
        _newSettings.heatctl.undershoot / 10,
        _newSettings.heatctl.undershoot % 10);
}

void MenuScreen::updatePageBoostIntval()
{
    char num[3] = { 0 };
    sprintf(num, "%2d", _newSettings.heatctl.boost_intval);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageCustomTempTimeout()
{
    char num[5] = { 0 };
    sprintf(num, "%4u", _newSettings.heatctl.custom_temp_timeout);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageTempCorrection()
{
    draw_temperature_value(20,
        _newSettings.heatctl.temp_correction / 10,
        _newSettings.heatctl.temp_correction % 10);
}

void MenuScreen::updatePageDisplayBrightness()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.display.brightness);
    Text::draw7Seg(num, 2, 20);

    Display::setContrast(_newSettings.display.brightness);
}

void MenuScreen::updatePageDisplayTimeout()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.display.timeout_secs);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageWifi()
{
    // Text::draw("CONNECTED: YES", 2, 0, 0, false);
    // Text::draw("NETWORK:", 4, 0, 0, false);
    // Text::draw("<SSID>", 5, 0, 0, false);

    wifi_screen_update();
}

void MenuScreen::drawPageTitle(const char* text)
{
    Text::draw(text, 0, 0, 0, false);
}

void MenuScreen::nextPage()
{
    if (static_cast<int>(_page) < static_cast<int>(Page::Last) - 1) {
        _page = static_cast<Page>(static_cast<int>(_page) + 1);
        draw();
    }
}

void MenuScreen::previousPage()
{
    if (_page > Page::First) {
        _page = static_cast<Page>(static_cast<int>(_page) - 1);
        draw();
    }
}

void MenuScreen::applySettings()
{
    settings = _newSettings;
    settings_save();
}

void MenuScreen::revertSettings()
{
    _newSettings = settings;

    Display::setContrast(settings.display.brightness);
}

void MenuScreen::adjustValue(int8_t amount)
{
    switch (_page) {
    case Page::HeatCtlMode:
        if (_newSettings.heatctl.mode == 0 && amount > 0) {
            _newSettings.heatctl.mode = 2;
        } else if (_newSettings.heatctl.mode == 2 && amount < 0) {
            _newSettings.heatctl.mode = 0;
        }
        updatePageHeatCtlMode();
        break;

    case Page::DaytimeTemp:
        _newSettings.heatctl.day_temp += amount;
        CLAMP_VALUE(_newSettings.heatctl.day_temp,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);
        updatePageDaytimeTemp();
        break;

    case Page::NightTimeTemp:
        _newSettings.heatctl.night_temp += amount;
        CLAMP_VALUE(_newSettings.heatctl.night_temp,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);
        updatePageNightTimeTemp();
        break;

    case Page::TempOvershoot:
        _newSettings.heatctl.overshoot += amount;
        CLAMP_VALUE(_newSettings.heatctl.overshoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        updatePageTempOvershoot();
        break;

    case Page::TempUndershoot:
        _newSettings.heatctl.undershoot += amount;
        CLAMP_VALUE(_newSettings.heatctl.undershoot,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
            SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
        updatePageTempUndershoot();
        break;

    case Page::BoostInterval:
        _newSettings.heatctl.boost_intval += amount;
        CLAMP_VALUE(_newSettings.heatctl.boost_intval,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN,
            SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX);
        updatePageBoostIntval();
        break;

    case Page::CustomTempTimeout:
        _newSettings.heatctl.custom_temp_timeout += amount;
        CLAMP_VALUE(_newSettings.heatctl.custom_temp_timeout,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN,
            SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX);
        updatePageCustomTempTimeout();
        break;

    case Page::DisplayBrightness:
        _newSettings.display.brightness += amount;
        updatePageDisplayBrightness();
        break;

    case Page::DisplayTimeout:
        _newSettings.display.timeout_secs += amount;
        updatePageDisplayTimeout();
        break;

    case Page::TempCorrection:
        _newSettings.heatctl.temp_correction += amount;
        CLAMP_VALUE(_newSettings.heatctl.temp_correction,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN,
            SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX);
        updatePageTempCorrection();
        break;

    // TODO dummy implementation
    case Page::WiFi:
        _page = Page::WIFI_PASSWORD;
        drawPageWifiPassword();
        break;

    case Page::Last:
    case Page::WIFI_PASSWORD:
        break;
    }
}