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

#include "DrawHelper.h"
#include "Extras.h"
#include "Graphics.h"
#include "HeatingController.h"
#include "Keypad.h"
#include "MenuScreen.h"
#include "Settings.h"
#include "TextInput.h"
#include "WifiScreen.h"

#include "display/Display.h"
#include "display/Text.h"

#include <stdio.h>

MenuScreen::MenuScreen(Settings& settings)
    : Screen("Menu")
    , _settings(settings)
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
#if 0
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
#endif
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

    case Page::Reboot:
        drawPageReboot();
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

void MenuScreen::drawPageReboot()
{
    drawPageTitle("REBOOT");
    _rebootCounter = 3;
    updatePageReboot();
}

void MenuScreen::drawPageWifi()
{
#if 0
    wifi_screen_init();
#endif
}

void MenuScreen::drawPageWifiPassword()
{
#if 0
    text_input_init(_wifiPsw, sizeof(_wifiPsw), "WiFi password:");
#endif
}

void MenuScreen::updatePageHeatCtlMode()
{
    // FIXME: proper mode name must be shown

    char num[4] = { 0 };
    sprintf(num, "%2d", _newSettings.HeatingController.Mode);

    Display::fillArea(0, 3, 128, 3, 0);

    switch (static_cast<HeatingController::Mode>(_newSettings.HeatingController.Mode)) {
    case HeatingController::Mode::Normal:
        graphics_draw_multipage_bitmap(graphics_calendar_icon_20x3p, 20, 3, 20, 2);
        Text::draw("NORMAL", 3, 50, 0, false);
        Text::draw("(SCHEDULE)", 4, 50, 0, false);
        break;

    case HeatingController::Mode::Off:
        graphics_draw_multipage_bitmap(graphics_off_icon_20x3p, 20, 3, 20, 2);
        Text::draw("OFF", 3, 50, 0, false);
        break;

    case HeatingController::Mode::Boost:
        break;
    }
}

void MenuScreen::updatePageDaytimeTemp()
{
    draw_temperature_value(20,
        _newSettings.HeatingController.DaytimeTemp / 10,
        _newSettings.HeatingController.DaytimeTemp % 10);
}

void MenuScreen::updatePageNightTimeTemp()
{
    draw_temperature_value(20,
        _newSettings.HeatingController.NightTimeTemp / 10,
        _newSettings.HeatingController.NightTimeTemp % 10);
}

void MenuScreen::updatePageTempOvershoot()
{
    draw_temperature_value(20,
        _newSettings.HeatingController.Overshoot / 10,
        _newSettings.HeatingController.Overshoot % 10);
}

void MenuScreen::updatePageTempUndershoot()
{
    draw_temperature_value(20,
        _newSettings.HeatingController.Undershoot / 10,
        _newSettings.HeatingController.Undershoot % 10);
}

void MenuScreen::updatePageBoostIntval()
{
    char num[4] = { 0 };
    sprintf(num, "%2d", _newSettings.HeatingController.BoostIntervalMins);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageCustomTempTimeout()
{
    char num[6] = { 0 };
    sprintf(num, "%4u", _newSettings.HeatingController.CustomTempTimeoutMins);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageTempCorrection()
{
    draw_temperature_value(20,
        _newSettings.HeatingController.TempCorrection / 10,
        _newSettings.HeatingController.TempCorrection% 10);
}

void MenuScreen::updatePageDisplayBrightness()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.Display.Brightness);
    Text::draw7Seg(num, 2, 20);

    Display::setContrast(_newSettings.Display.Brightness);
}

void MenuScreen::updatePageDisplayTimeout()
{
    char num[4] = { 0 };
    sprintf(num, "%3d", _newSettings.Display.TimeoutSecs);
    Text::draw7Seg(num, 2, 20);
}

void MenuScreen::updatePageReboot()
{
    Text::draw("Press the (+) button", 2, 5, 0, 0);

    char s[] = "0 time(s) to reboot.";
    s[0] = '0' + _rebootCounter;
    Text::draw(s, 3, 5, 0, 0);
}

void MenuScreen::updatePageWifi()
{
    // Text::draw("CONNECTED: YES", 2, 0, 0, false);
    // Text::draw("NETWORK:", 4, 0, 0, false);
    // Text::draw("<SSID>", 5, 0, 0, false);

#if 0
    wifi_screen_update();
#endif
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
    _settings.data = _newSettings;
    _settings.save();
}

void MenuScreen::revertSettings()
{
    _newSettings = _settings.data;

    Display::setContrast(_settings.data.Display.Brightness);
}

void MenuScreen::adjustValue(int8_t amount)
{
    switch (_page) {
    case Page::HeatCtlMode:
        if (_newSettings.HeatingController.Mode == 0 && amount > 0) {
            _newSettings.HeatingController.Mode = 2;
        } else if (_newSettings.HeatingController.Mode == 2 && amount < 0) {
            _newSettings.HeatingController.Mode = 0;
        }
        updatePageHeatCtlMode();
        break;

    case Page::DaytimeTemp:
        _newSettings.HeatingController.DaytimeTemp = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.DaytimeTemp,
            amount,
            Limits::HeatingController::DaytimeTempMin,
            Limits::HeatingController::DaytimeTempMax
        );
        updatePageDaytimeTemp();
        break;

    case Page::NightTimeTemp:
        _newSettings.HeatingController.NightTimeTemp = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.NightTimeTemp,
            amount,
            Limits::HeatingController::NightTimeTempMin,
            Limits::HeatingController::NightTimeTempMax
        );
        updatePageNightTimeTemp();
        break;

    case Page::TempOvershoot:
        _newSettings.HeatingController.Overshoot = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.Overshoot,
            amount,
            Limits::HeatingController::TempOvershootMin,
            Limits::HeatingController::TempOvershootMax
        );
        updatePageTempOvershoot();
        break;

    case Page::TempUndershoot:
        _newSettings.HeatingController.Undershoot = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.Undershoot,
            amount,
            Limits::HeatingController::TempUndershootMin,
            Limits::HeatingController::TempUndershootMax
        );
        updatePageTempUndershoot();
        break;

    case Page::BoostInterval:
        _newSettings.HeatingController.BoostIntervalMins = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.BoostIntervalMins,
            amount,
            Limits::HeatingController::BoostIntervalMin,
            Limits::HeatingController::BoostIntervalMax
        );
        updatePageBoostIntval();
        break;

    case Page::CustomTempTimeout:
        _newSettings.HeatingController.CustomTempTimeoutMins = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.CustomTempTimeoutMins,
            amount,
            Limits::HeatingController::CustomTempTimeoutMin,
            Limits::HeatingController::CustomTempTimeoutMax
        );
        updatePageCustomTempTimeout();
        break;

    case Page::DisplayBrightness:
        _newSettings.Display.Brightness += amount;
        updatePageDisplayBrightness();
        break;

    case Page::DisplayTimeout:
        _newSettings.Display.TimeoutSecs += amount;
        updatePageDisplayTimeout();
        break;

    case Page::TempCorrection:
        _newSettings.HeatingController.TempCorrection = Extras::adjustValueWithRollOver(
            _newSettings.HeatingController.TempCorrection,
            amount,
            Limits::HeatingController::TempCorrectionMin,
            Limits::HeatingController::TempCorrectionMax
        );
        updatePageTempCorrection();
        break;

    case Page::Reboot:
        if (amount > 0 && --_rebootCounter == 0) {
            _log.warning_p(PSTR("initiating manual reboot"));
            ESP.restart();
        }
        updatePageReboot();
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