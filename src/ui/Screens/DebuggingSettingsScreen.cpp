#include "DebuggingSettingsScreen.h"
#include "Utilities.h"

#include "../Model.h"

using namespace UI;
using namespace std::string_view_literals;

namespace
{
    char _logLevelValueLabel[6]{};
}

DebuggingSettingsScreen::DebuggingSettingsScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::DebuggingSettingsScreen, model, graphics }
    , _menu{
        graphics,
        1,
        7,
        MenuItem{ "Log Level", _logLevelValueLabel },
        MenuItem{ "[Reboot]" },
        MenuItem{
            "Firmware Ver.",
            [&] {
                static char s[12]{};
                const auto [major, minor, patch] = model.firmwareVersion.parts();
                snprintf(s, sizeof(s), "%u.%u.%u", major, minor, patch);
                return s;
            }()
        },
        MenuItem{
            "IoT Base Ver.",
            [&] {
                static char s[12]{};
                const auto [major, minor, patch] = model.baseVersion.parts();
                snprintf(s, sizeof(s), "%u.%u.%u", major, minor, patch);
                return s;
            }()
        }
    }
{}

void DebuggingSettingsScreen::activate()
{
    graphics().drawText(0, 0, "Debugging Settings"sv, Resources::Fonts::Oled);

    updateValueLabels();
    _menu.reset();
    _menu.update();
}

void DebuggingSettingsScreen::update()
{
}

Screen::Result DebuggingSettingsScreen::handleKeyPress(const Keypad::Keys keys)
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

Screen::Result DebuggingSettingsScreen::selectMenuItem() const
{
    switch (_menu.currentIndex()) {
        case 1:
            system_restart();
            break;

        default:
            break;
    }

    return Result{};
}

void DebuggingSettingsScreen::stepSelectedSetting(const StepDirection direction)
{
    switch (_menu.currentIndex()) {
        case 0:
            model().settings.system.maximumLogLevel = stepValue(
                model().settings.system.maximumLogLevel,
                direction,
                static_cast<uint8_t>(Log::Severity::Error),
                static_cast<uint8_t>(Log::Severity::Debug)
            );
            break;

        default:
            return;
    }

    updateValueLabels();
    _menu.updateSelectedItem();
}

void DebuggingSettingsScreen::updateValueLabels()
{
    snprintf(
        _logLevelValueLabel,
        sizeof(_logLevelValueLabel),
        "%s",
        [&] {
            switch (model().settings.system.maximumLogLevel) {
                case 0:
                    return "Error";
                case 1:
                    return "Warn.";
                case 2:
                    return "Info";
                case 3:
                    return "Debug";
            }
            return "Unkn.";
        }()
    );
}
