#include "ZoneSettingsScreen.h"
#include "Utilities.h"

#include "../Model.h"

#include <string>

using namespace UI;
using namespace std::string_view_literals;

namespace
{
    char _modeValueLabel[8]{};
    char _highTargetValueLabel[16]{};
    char _lowTargetValueLabel[16]{};
    char _holidayTargetValueLabel[16]{};
    char _boostInitialDurationValueLabel[11]{};
    char _boostExtensionDurationValueLabel[11]{};
    char _overrideTimeoutValueLabel[11]{};
    char _heatingStartDelayValueLabel[11]{};
    char _windowLockoutDurationValueLabel[11]{};
    char _heatingOvershootValueLabel[16]{};
    char _heatingUndershootValueLabel[16]{};
}

ZoneSettingsScreen::ZoneSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "[Schedule]" },
        MenuItem{ "Mode", _modeValueLabel },
        MenuItem{ "High Target", _highTargetValueLabel },
        MenuItem{ "Low Target", _lowTargetValueLabel },
        MenuItem{ "Holiday Tgt.", _holidayTargetValueLabel },
        MenuItem{ "Bst.Init.Dur.", _boostInitialDurationValueLabel },
        MenuItem{ "Bst.Ext.Dur.", _boostExtensionDurationValueLabel },
        MenuItem{ "Ovrrd. T.out.", _overrideTimeoutValueLabel },
        MenuItem{ "Heat.Strt.Dly.", _heatingStartDelayValueLabel },
        MenuItem{ "Wnd.Lckout.Dur.", _windowLockoutDurationValueLabel },
        MenuItem{ "Oversht.Tmp.", _heatingOvershootValueLabel },
        MenuItem{ "Undersht.Tmp.", _heatingUndershootValueLabel },
    }
{}

void ZoneSettingsScreen::activate()
{
    auto x = graphics().drawText(0, 0, "Zone Settings: "sv, Resources::Fonts::Oled);
    graphics().drawText(x, 0, std::to_string(model().navigation.selectedZoneIndex), Resources::Fonts::Oled);

    updateValueLabels();
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
    } else if (keys & Keys::Plus) {
        stepSelectedSetting(StepDirection::Up);
    } else if (keys & Keys::Minus) {
        stepSelectedSetting(StepDirection::Down);
    }

    return Result{};
}

Screen::Result ZoneSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        case 0:
            return Navigate{ .id = ScreenID::ZoneSchedule };
        default:
            break;
    }

    return Result{};
}

void ZoneSettingsScreen::stepSelectedSetting(const StepDirection direction)
{
    auto& zone = model().settings.heating.zones[model().navigation.selectedZoneIndex];

    switch (_menu.currentIndex()) {
        case 1: {
            zone.state.mode = static_cast<HeatingZoneController::Mode>(
                stepValue(
                    static_cast<int>(zone.state.mode),
                    direction,
                    static_cast<int>(HeatingZoneController::Mode::Off),
                    static_cast<int>(HeatingZoneController::Mode::Holiday)
                )
            );
            break;
        }

        case 2:
            zone.state.highTargetTemperature = stepValue(
                zone.state.highTargetTemperature,
                direction,
                100,
                300
            );
            break;

        case 3:
            zone.state.lowTargetTemperature = stepValue(
                zone.state.lowTargetTemperature,
                direction,
                100,
                300
            );
            break;

        case 4:
            zone.config.holidayModeTemperature = stepValue(
                zone.config.holidayModeTemperature,
                direction,
                100,
                300
            );
            break;

        case 5:
            zone.config.boostInitialDurationSeconds = stepValue(
                zone.config.boostInitialDurationSeconds,
                direction,
                5 * 60u,
                60 * 60u,
                60u
            );
            break;

        case 6:
            zone.config.boostExtensionDurationSeconds = stepValue(
                zone.config.boostExtensionDurationSeconds,
                direction,
                5 * 60u,
                60 * 60u,
                60u
            );
            break;

        case 7:
            zone.config.overrideTimeoutSeconds = stepValue(
                zone.config.overrideTimeoutSeconds,
                direction,
                30 * 60u,
                180 * 60u,
                10 * 60u
            );
            break;

        case 8:
            zone.config.heatingStartDelaySeconds = stepValue(
                zone.config.heatingStartDelaySeconds,
                direction,
                0 * 60u,
                60 * 60u,
                60u
            );
            break;

        case 9:
            zone.config.openWindowLockoutDurationSeconds = stepValue(
                zone.config.openWindowLockoutDurationSeconds,
                direction,
                0 * 60u,
                60 * 60u,
                60u
            );
            break;

        case 10:
            zone.config.heatingOvershoot = stepValue(
                zone.config.heatingOvershoot,
                direction,
                0,
                100
            );
            break;

        case 11:
            zone.config.heatingUndershoot = stepValue(
                zone.config.heatingUndershoot,
                direction,
                0,
                100
            );
            break;

        default:
            return;
    }

    updateValueLabels();
    _menu.updateSelectedItem();
}

void ZoneSettingsScreen::updateValueLabels()
{
    auto& zone = model().settings.heating.zones[model().navigation.selectedZoneIndex];

    snprintf(
        _modeValueLabel,
        sizeof(_modeValueLabel),
        "%s",
        [&] {
            switch (zone.state.mode) {
                case HeatingZoneController::Mode::Off:
                    return "Off";
                case HeatingZoneController::Mode::Auto:
                    return "Auto";
                case HeatingZoneController::Mode::Holiday:
                    return "Holiday";
            }
            return "Unknown";
        }()
    );

    formatTemperatureValue(_highTargetValueLabel, zone.state.highTargetTemperature);
    formatTemperatureValue(_lowTargetValueLabel, zone.state.lowTargetTemperature);
    formatTemperatureValue(_holidayTargetValueLabel, zone.config.holidayModeTemperature);
    formatValueWithSuffix(_boostInitialDurationValueLabel, zone.config.boostInitialDurationSeconds / 60u, 'm');
    formatValueWithSuffix(_boostExtensionDurationValueLabel, zone.config.boostExtensionDurationSeconds / 60u, 'm');
    formatValueWithSuffix(_overrideTimeoutValueLabel, zone.config.overrideTimeoutSeconds / 60u, 'm');
    formatValueWithSuffix(_heatingStartDelayValueLabel, zone.config.heatingStartDelaySeconds / 60u, 'm');
    formatValueWithSuffix(_windowLockoutDurationValueLabel, zone.config.openWindowLockoutDurationSeconds / 60u, 'm');
    formatTemperatureValue(_heatingOvershootValueLabel, zone.config.heatingOvershoot);
    formatTemperatureValue(_heatingUndershootValueLabel, zone.config.heatingUndershoot);
}
