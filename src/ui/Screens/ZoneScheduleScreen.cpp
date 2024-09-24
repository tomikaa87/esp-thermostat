#include "ZoneScheduleScreen.h"

#include "../Model.h"

#include <string>

using namespace UI;
using namespace std::string_view_literals;

ZoneScheduleScreen::ZoneScheduleScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSchedule, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "" }
    }
{}

void ZoneScheduleScreen::activate()
{
    auto x = graphics().drawText(0, 0, "Schedule: "sv, Resources::Fonts::Oled);
    graphics().drawText(x, 0, std::to_string(model().navigation.selectedZoneIndex), Resources::Fonts::Oled);

    _menu.reset();
    _menu.update();
}

void ZoneScheduleScreen::update()
{
}

Screen::Result ZoneScheduleScreen::handleKeyPress(const Keypad::Keys keys)
{
    using Keys = Keypad::Keys;

    if (keys & Keys::Plus) {
        _menu.step(UI::StepDirection::Up);
        _menu.update();
    } else if (keys & Keys::Minus) {
        _menu.step(UI::StepDirection::Down);
        _menu.update();
    } else if (keys & Keys::Menu) {
        if (!(keys & Keys::LongPress)) {
            return Navigate{ .id = ScreenID::ZoneSettings };
        }
    } else if (keys & Keys::Boost) {
        return selectMenuItem();
    }

    return Result{};
}

Screen::Result ZoneScheduleScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        default:
            break;
    }

    return Result{};
}