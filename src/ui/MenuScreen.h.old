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

#pragma once

#include "Keypad.h"
#include "Screen.h"
#include "Settings.h"

#include <Logger.h>

#include <cstdint>

class MenuScreen : public Screen
{
public:
    MenuScreen(Settings& settings);

    void activate() override;
    void update() override;
    Action keyPress(Keypad::Keys keys) override;

private:
    Logger _log{ "MenuScreen" };
    Settings& _settings;
    Settings::Data _newSettings;
    char _wifiPsw[64] = { 0 };

    uint8_t _rebootCounter = 3;

    enum class Page
    {
        First = 0,

        HeatCtlMode = First,
        DaytimeTemp,
        NightTimeTemp,
        TempOvershoot,
        TempUndershoot,
        BoostInterval,
        CustomTempTimeout,
        DisplayBrightness,
        DisplayTimeout,
        TempCorrection,
        Reboot,

        // These pages cannot be accessed by normal navigation
        WiFi,
        WIFI_PASSWORD,

        Last = Reboot
    } _page = Page::First;

    void draw();

    void drawPageHeatCtlMode();
    void drawPageDaytimeTemp();
    void drawPageNightTimeTemp();
    void drawPageTempOvershoot();
    void drawPageTempUndershoot();
    void drawPageBoostIntval();
    void drawPageCustomTempTimeout();
    void drawPageDisplayBrightness();
    void drawPageDisplayTimeout();
    void drawPageTempCorrection();
    void drawPageReboot();
    void drawPageWifi();
    void drawPageWifiPassword();
    void updatePageHeatCtlMode();
    void updatePageDaytimeTemp();
    void updatePageNightTimeTemp();
    void updatePageTempOvershoot();
    void updatePageTempUndershoot();
    void updatePageBoostIntval();
    void updatePageCustomTempTimeout();
    void updatePageTempCorrection();
    void updatePageDisplayBrightness();
    void updatePageDisplayTimeout();
    void updatePageReboot();
    void updatePageWifi();
    void drawPageTitle(const char* text);
    void nextPage();
    void previousPage();
    void applySettings();
    void revertSettings();
    void adjustValue(int8_t amount);
};
