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
    Created on 2017-01-07
*/

#include "DrawHelper.h"
#include "Graphics.h"
#include "Keypad.h"
#include "SchedulingScreen.h"
#include "SystemClock.h"
#include "main.h"

#include "display/Display.h"
#include "display/Text.h"

#include <cstring>

SchedulingScreen::SchedulingScreen(Settings& settings, const ISystemClock& systemClock)
    : Screen("Scheduling")
    , _settings(settings)
    , _systemClock(systemClock)
{
    memset(_daysData, 0, sizeof(_daysData));
}

void SchedulingScreen::activate()
{
    const auto localTime = _systemClock.localTime();
    struct tm* t = gmtime(&localTime);
    _day = t->tm_wday;
    _intvalIdx = 0;
    memcpy(_daysData, _settings.data.Scheduler.DayData, sizeof(Settings::SchedulerDayData) * 7);
    draw();
}

void SchedulingScreen::update()
{
}

Screen::Action SchedulingScreen::keyPress(Keypad::Keys keys)
{
    // 1: advance 15 minutes (long: go back 15 minutes)
    // 2: advance 1 day (long: go back 1 day)
    // 3: save and exit
    // 4: cancel
    // 5: set nighttime mode + advance 15 minutes
    // 6: set daytime mode + advance 15 minutes

    if (keys & Keypad::Keys::Plus) {
        setModeAndAdvance(true);
        // if (keys & KEY_LONG_PRESS)
        // 	prev_interval();
        // else
        // 	next_interval();
    } else if (keys & Keypad::Keys::Minus) {
        setModeAndAdvance(false);
        // if (keys & KEY_LONG_PRESS)
        // 	prev_day();
        // else
        // 	next_day();
    } else if (keys & Keypad::Keys::Menu) {
        if (++_menuPressCnt == 2) {
            _menuPressCnt = 0;
            if (!(keys & Keypad::Keys::LongPress)) {
                applyChanges();
            }
            return Action::NavigateBack;
        }
    } else if (keys & Keypad::Keys::Boost) {
        nextDay();
        // return UI_RESULT_SWITCH_MAIN_SCREEN;
    } else if (keys & Keypad::Keys::Left) {
        prevInterval();
        // set_mode_and_advance(false);
    } else if (keys & Keypad::Keys::Right) {
        nextInterval();
        // set_mode_and_advance(true);
    }

    return Action::NoAction;
}

void SchedulingScreen::draw()
{
    Display::clear();

    drawDayName();
    drawIntervalDisplay();
    drawIntervalIndicator();
    updateScheduleBar();
}

void SchedulingScreen::drawDayName()
{
    draw_weekday(0, _day);
}

void SchedulingScreen::drawIntervalDisplay()
{
    uint8_t hours = _intvalIdx >> 1;
    uint8_t mins = (_intvalIdx & 1) * 30;

    char s[6] = { 0 };
    sprintf(s, "%02u %02u", hours, mins);

    Text::draw7Seg(s, 2, 29);
}

void SchedulingScreen::drawIntervalIndicator()
{
    draw_schedule_indicator(_intvalIdx);
}

void SchedulingScreen::updateScheduleBar()
{
    draw_schedule_bar(_daysData[_day]);
}

void SchedulingScreen::setModeAndAdvance(bool daytime)
{
    uint8_t bit_idx = _intvalIdx & 0b111;
    uint8_t byte_idx = _intvalIdx >> 3;
    uint8_t mask = 1 << bit_idx;

    if (daytime)
        _daysData[_day][byte_idx] |= mask;
    else
        _daysData[_day][byte_idx] &= ~mask;

    ++_intvalIdx;
    if (_intvalIdx > 47)
        _intvalIdx = 0;

    drawIntervalIndicator();
    drawIntervalDisplay();
    updateScheduleBar();
}

void SchedulingScreen::nextInterval()
{
    if (_intvalIdx < 47) {
        ++_intvalIdx;
        drawIntervalDisplay();
        drawIntervalIndicator();
    }
}

void SchedulingScreen::prevInterval()
{
    if (_intvalIdx > 0) {
        --_intvalIdx;
        drawIntervalDisplay();
        drawIntervalIndicator();
    }
}

void SchedulingScreen::nextDay()
{
    ++_day;
    if (_day > 6)
        _day = 0;

    _intvalIdx = 0;
    draw();
}

void SchedulingScreen::prevDay()
{
    --_day;
    if (_day > 6)
        _day = 6;

    _intvalIdx = 0;
    draw();
}

void SchedulingScreen::applyChanges()
{
    memcpy(_settings.data.Scheduler.DayData, _daysData, sizeof(Settings::SchedulerDayData) * 7);
    _settings.save();
}