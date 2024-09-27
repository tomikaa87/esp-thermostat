#include "DisplaySettingsScreen.h"

#include "../Model.h"

using namespace UI;
using namespace std::string_view_literals;

namespace
{
    // FIXME There's a potential bug in extensa GCC, and because of that, it can't take the proper address of a char array until something is written into it
    char _brightnessValueLabel[4]{};
    char _timeoutValueLabel[6]{};
}

DisplaySettingsScreen::DisplaySettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::DisplaySettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "Brightness", _brightnessValueLabel },
        MenuItem{ "Timeout", _timeoutValueLabel }
    }
{}

void DisplaySettingsScreen::activate()
{
    graphics().drawText(0, 0, "Display Settings"sv, Resources::Fonts::Oled);

    updateValueLabels();
    _menu.reset();
    _menu.update();
}

void DisplaySettingsScreen::update()
{
}

Screen::Result DisplaySettingsScreen::handleKeyPress(const Keypad::Keys keys)
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
    } else if (keys & Keys::Plus) {
        stepSelectedSetting(StepDirection::Up);
    } else if (keys & Keys::Minus) {
        stepSelectedSetting(StepDirection::Down);
    }

    return Result{};
}

Screen::Result DisplaySettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        default:
            break;
    }

    return Result{};
}

void DisplaySettingsScreen::stepSelectedSetting(const StepDirection direction)
{
    switch (_menu.currentIndex()) {
        case 0:
            model().settings.system.display.brightness += direction == StepDirection::Up ? 1 : -1;
            break;

        case 1:
            model().settings.system.display.timeoutSecs += direction == StepDirection::Up ? 1 : -1;
            break;

        default:
            return;
    }

    updateValueLabels();
    _menu.updateSelectedItem();
}

void DisplaySettingsScreen::updateValueLabels()
{
    snprintf(
        _brightnessValueLabel,
        sizeof(_brightnessValueLabel),
        "%u",
        model().settings.system.display.brightness
    );

    snprintf(
        _timeoutValueLabel,
        sizeof(_timeoutValueLabel),
        "%u s",
        model().settings.system.display.timeoutSecs
    );
}
