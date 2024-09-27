#include "ZoneSettingsScreen.h"

#include "../Model.h"

#include <limits>
#include <string>

using namespace UI;
using namespace std::string_view_literals;

namespace
{
    char _modeValueLabel[8]{};
    char _highTargetValueLabel[16]{};
    char _lowTargetValueLabel[16]{};
    char _holidayTargetValueLabel[16]{};
}

namespace
{
    template <typename CharArray, typename Value>
    void formatTemperatureValue(CharArray& s, const Value value)
    {
        snprintf(
            s,
            sizeof(s),
            "%d.%d C",
            value / 10,
            value % 10
        );
    }
}

namespace
{
    template <typename ValueType>
    ValueType stepValue(
        const ValueType value,
        const StepDirection direction,
        const ValueType min = std::numeric_limits<ValueType>::min(),
        const ValueType max = std::numeric_limits<ValueType>::max()
    )
    {
        if (direction == StepDirection::Up) {
            if (value < max) {
                return value + 1;
            } else {
                return min;
            }
        } else {
            if (value > min) {
                return value - 1;
            } else {
                return max;
            }
        }
    }
}

ZoneSettingsScreen::ZoneSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::ZoneSettings, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "Schedule..." },
        MenuItem{ "Mode", _modeValueLabel },
        MenuItem{ "High Target", _highTargetValueLabel },
        MenuItem{ "Low Target", _lowTargetValueLabel },
        MenuItem{ "Holiday Tgt.", _holidayTargetValueLabel },
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
        case 1:
            zone.state.mode = static_cast<HeatingZoneController::Mode>(
                (static_cast<unsigned>(zone.state.mode) + (direction == StepDirection::Up ? 1 : -1)) & 3
            );
            break;

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
}
