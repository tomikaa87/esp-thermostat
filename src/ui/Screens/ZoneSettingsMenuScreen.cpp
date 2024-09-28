#include "ZoneSettingsMenuScreen.h"

#include "../Model.h"

using namespace UI;
using namespace std::string_view_literals;

ZoneSettingsMenuScreen::ZoneSettingsMenuScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSettingsMenu, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        // FIXME use dynamic zone names to match HeatingZone objects
        MenuItem{ "[Zone 0]" },
        MenuItem{ "[Zone 1]" },
        MenuItem{ "[Zone 2]" },
        MenuItem{ "[Zone 3]" },
        MenuItem{ "[Zone 10]" },
        MenuItem{ "[Zone 11]" }
    }
{}

void ZoneSettingsMenuScreen::activate()
{
    graphics().drawText(0, 0, "Zone Settings", Resources::Fonts::Oled);

    _menu.reset();
    _menu.update();
}

void ZoneSettingsMenuScreen::update()
{
}

Screen::Result ZoneSettingsMenuScreen::handleKeyPress(const Keypad::Keys keys)
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
            return Navigate{ .id = ScreenID::MainMenu };
        }
    } else if (keys & Keys::Boost) {
        model().navigation.selectedZoneIndex = _menu.currentIndex();
        return Navigate{ .id = ScreenID::ZoneSettings };
    }

    return Result{};
}
