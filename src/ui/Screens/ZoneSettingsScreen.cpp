#include "ZoneSettingsScreen.h"

#include "../Model.h"

#include <string>

using namespace UI;
using namespace std::string_view_literals;

ZoneSettingsScreen::ZoneSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "General Settings" },
        MenuItem{ "Schedule" },
    }
{}

void ZoneSettingsScreen::activate()
{
    auto x = graphics().drawText(0, 0, "Zone Settings: "sv, Resources::Fonts::Oled);
    graphics().drawText(x, 0, std::to_string(model().navigation.selectedZoneIndex), Resources::Fonts::Oled);

    _menu.reset();
    _menu.update();
}

void ZoneSettingsScreen::update()
{
}

Screen::Result ZoneSettingsScreen::handleKeyPress(const Keypad::Keys keys)
{
    using Keys = Keypad::Keys;

    if (keys & Keys::Left) {
        _menu.step(UI::StepDirection::Up);
        _menu.update();
    } else if (keys & Keys::Right) {
        _menu.step(UI::StepDirection::Down);
        _menu.update();
    } else if (keys & Keys::Menu) {
        if (!(keys & Keys::LongPress)) {
            return Navigate{ .id = ScreenID::ZoneSettingsMenu };
        }
    } else if (keys & Keys::Boost) {
        return selectMenuItem();
    }

    return Result{};
}

Screen::Result ZoneSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        case 0:
            return Navigate{ .id = ScreenID::ZoneGeneralSettings };
        case 1:
            return Navigate{ .id = ScreenID::ZoneSchedule };
        default:
            break;
    }

    return Result{};
}