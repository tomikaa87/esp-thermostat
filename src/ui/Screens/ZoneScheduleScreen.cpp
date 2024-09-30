#include "ZoneScheduleScreen.h"

#include "../Graphics.h"
#include "../Model.h"

#include <string>

using namespace UI;
using namespace std::string_view_literals;

ZoneScheduleScreen::ZoneScheduleScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSchedule, model, graphics }
{}

void ZoneScheduleScreen::activate()
{
    auto x = graphics().drawText(0, 0, "Schedule: "sv, Resources::Fonts::Oled);
    graphics().drawText(x, 0, std::to_string(model().navigation.selectedZoneIndex), Resources::Fonts::Oled);

    drawScheduleBar();
    drawSchedulePositionIndicator();
    drawWeekday();
}

void ZoneScheduleScreen::update()
{
}

Screen::Result ZoneScheduleScreen::handleKeyPress(const Keypad::Keys keys)
{
    using Keys = Keypad::Keys;

    if (keys & Keys::Plus) {
        setScheduleBit(true);
        drawScheduleBar();
        stepSchedulePosition(StepDirection::Up);
    } else if (keys & Keys::Minus) {
        setScheduleBit(false);
        drawScheduleBar();
        stepSchedulePosition(StepDirection::Up);
    } else if (keys & Keys::Menu) {
        if (!(keys & Keys::LongPress)) {
            return Navigate{ .id = ScreenID::ZoneSettings };
        }
    } else if (keys & Keys::Boost) {
        if (++_scheduleDay > 6) {
            _scheduleDay = 0;
        }
        _scheduleBitIndex = 0;
        drawWeekday();
        drawScheduleBar();
        drawSchedulePositionIndicator();
    } else if (keys & Keys::Left) {
        stepSchedulePosition(StepDirection::Down);
    } else if (keys & Keys::Right) {
        stepSchedulePosition(StepDirection::Up);
    }

    return Result{};
}

void ZoneScheduleScreen::drawScheduleBar()
{
    graphics().drawScheduleBar(
        model().settings.heating.zones[model().navigation.selectedZoneIndex].schedule,
        _scheduleDay * 6
    );
}

void ZoneScheduleScreen::drawSchedulePositionIndicator()
{
    graphics().drawScheduleBarPositionIndicator(_scheduleBitIndex);

    auto mins = (_scheduleBitIndex & 1) * 30;
    auto hours = _scheduleBitIndex >> 1;

    char buf[14]{};
    snprintf(buf, sizeof(buf), "%2u:%02u", hours, mins);

    const auto x = graphics().drawText(0, 3, "Time: "sv, Resources::Fonts::Oled);
    graphics().drawText(x, 3, buf, Resources::Fonts::Oled);
}

void ZoneScheduleScreen::drawWeekday()
{
    const auto x = graphics().drawText(0, 2, "Day: "sv, Resources::Fonts::Oled);
    graphics().drawShortWeekday(x, 2, _scheduleDay);
}

void ZoneScheduleScreen::stepSchedulePosition(const StepDirection direction)
{
    switch (direction) {
        case StepDirection::Down:
            if (_scheduleBitIndex > 0) {
                --_scheduleBitIndex;
                drawSchedulePositionIndicator();
            }
            break;

        case StepDirection::Up:
            if (_scheduleBitIndex < 47) {
                ++_scheduleBitIndex;
                drawSchedulePositionIndicator();
            }
            break;
    }
}

void ZoneScheduleScreen::setScheduleBit(const bool on)
{
    auto& schedule = model().settings.heating.zones[model().navigation.selectedZoneIndex].schedule;
    const auto byteOffset = _scheduleDay * 6u + _scheduleBitIndex / 8u;
    const auto bitMask = 1 << (7 - _scheduleBitIndex % 8); // MSB-first

    if (on) {
        schedule[byteOffset] |= bitMask;
    } else {
        schedule[byteOffset] &= ~bitMask;
    }
}