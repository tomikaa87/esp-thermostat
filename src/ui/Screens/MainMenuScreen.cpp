#include "MainMenuScreen.h"
#include "Utilities.h"

#include "../Model.h"

#include <LogSeverity.h>

using namespace UI;
using namespace std::string_view_literals;

namespace
{
}

MainMenuScreen::MainMenuScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::MainMenu, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "[Zone Settings]" },
        MenuItem{ "[General Settings]" },
        MenuItem{ "[Display Settings]" },
        MenuItem{ "[Date/Time Sett.]" },
        MenuItem{ "[Debugging Sett.]" },
    }
{}

void MainMenuScreen::activate()
{
    graphics().drawText(0, 0, "Main Menu"sv, Resources::Fonts::Oled);

    updateValueLabels();
    _menu.reset();
    _menu.update();
}

void MainMenuScreen::update()
{
}

Screen::Result MainMenuScreen::handleKeyPress(const Keypad::Keys keys)
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
            return Navigate{ .id = ScreenID::Main };
        }
    } else if (keys & Keys::Boost) {
        return selectMenuItem();
    } else if (keys & Keys::Plus) {
        stepSelectedSetting(StepDirection::Up);
    } else if (keys & Keys::Minus) {
        stepSelectedSetting(StepDirection::Down);
    }

    return Result{};
}

Screen::Result MainMenuScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        case 0:
            return Navigate{ .id = ScreenID::ZoneSettingsMenu };
        case 1:
            return Navigate{ .id = ScreenID::GeneralSettings };
        case 2:
            return Navigate{ .id = ScreenID::DisplaySettings };
        case 3:
            return Navigate{ .id = ScreenID::DateTimeSettings };
        case 4:
            return Navigate{ .id = ScreenID::DebuggingSettingsScreen };
        default:
            break;
    }

    return Result{};
}

void MainMenuScreen::stepSelectedSetting(const StepDirection direction)
{
    switch (_menu.currentIndex()) {
        default:
            return;
    }

    updateValueLabels();
    _menu.updateSelectedItem();
}

void MainMenuScreen::updateValueLabels()
{
}