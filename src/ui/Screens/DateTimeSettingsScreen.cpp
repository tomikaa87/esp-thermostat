#include "DateTimeSettingsScreen.h"

using namespace UI;
using namespace std::string_view_literals;

DateTimeSettingsScreen::DateTimeSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::DateTimeSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "" }
    }
{}

void DateTimeSettingsScreen::activate()
{
    graphics().drawText(0, 0, "Date/Time Settings"sv, Resources::Fonts::Oled);

    _menu.reset();
    _menu.update();
}

void DateTimeSettingsScreen::update()
{
}

Screen::Result DateTimeSettingsScreen::handleKeyPress(const Keypad::Keys keys)
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
        return selectMenuItem();
    }

    return Result{};
}

Screen::Result DateTimeSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        default:
            break;
    }

    return Result{};
}