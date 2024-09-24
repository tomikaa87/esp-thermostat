#include "GeneralSettingsScreen.h"

using namespace UI;
using namespace std::string_view_literals;

GeneralSettingsScreen::GeneralSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::GeneralSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "" }
    }
{}

void GeneralSettingsScreen::activate()
{
    graphics().drawText(0, 0, "General Settings"sv, Resources::Fonts::Oled);

    _menu.reset();
    _menu.update();
}

void GeneralSettingsScreen::update()
{
}

Screen::Result GeneralSettingsScreen::handleKeyPress(const Keypad::Keys keys)
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
            return Navigate{ .id = ScreenID::MainMenu };
        }
    } else if (keys & Keys::Boost) {
        return selectMenuItem();
    }

    return Result{};
}

Screen::Result GeneralSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        default:
            break;
    }

    return Result{};
}