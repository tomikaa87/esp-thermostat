#include "GeneralSettingsScreen.h"

#include "../Model.h"

using namespace UI;
using namespace std::string_view_literals;

namespace
{
    char _masterEnableValueLabel[4]{};
    char _energySaverValueLabel[4]{};
}

GeneralSettingsScreen::GeneralSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::GeneralSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "Master Enable", _masterEnableValueLabel },
        MenuItem{ "Energy Optim.", _energySaverValueLabel }
    }
{}

void GeneralSettingsScreen::activate()
{
    graphics().drawText(0, 0, "General Settings"sv, Resources::Fonts::Oled);

    updateValueLabels();
    _menu.reset();
    _menu.update();
}

void GeneralSettingsScreen::update()
{
}

Screen::Result GeneralSettingsScreen::handleKeyPress(const Keypad::Keys keys)
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

Screen::Result GeneralSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        default:
            break;
    }

    return Result{};
}

void GeneralSettingsScreen::stepSelectedSetting(const StepDirection direction)
{
    switch (_menu.currentIndex()) {
        case 0:
            model().settings.system.masterEnable ^= true;
            break;

        case 1:
            model().settings.system.energyOptimizerEnabled ^= true;
            break;

        default:
            return;
    }

    updateValueLabels();
    _menu.updateSelectedItem();
}

void GeneralSettingsScreen::updateValueLabels()
{
    snprintf(
        _masterEnableValueLabel,
        sizeof(_masterEnableValueLabel),
        "%s",
        model().settings.system.masterEnable ? "On" : "Off"
    );

    snprintf(
        _energySaverValueLabel,
        sizeof(_energySaverValueLabel),
        "%s",
        model().settings.system.energyOptimizerEnabled ? "On" : "Off"
    );
}
