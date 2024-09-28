#include "MainMenuScreen.h"

using namespace UI;
using namespace std::string_view_literals;

MainMenuScreen::MainMenuScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::MainMenu, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "[Zone Settings]" },
        MenuItem{ "[General Settings]" },
        MenuItem{ "[Display Settings]" },
        MenuItem{ "[Date/Time Sett.]" }
    }
{}

void MainMenuScreen::activate()
{
    graphics().drawText(0, 0, "Main Menu"sv, Resources::Fonts::Oled);

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
        default:
            break;
    }

    return Result{};
}