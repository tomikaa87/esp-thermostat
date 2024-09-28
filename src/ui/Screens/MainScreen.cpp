#include "MainScreen.h"

#include "../Menu.h"

#include <cstdio>
#include <string_view>

using namespace std::string_view_literals;

using namespace UI;

namespace Positions
{
    struct Position
    {
        unsigned x{};
        unsigned line{};
    };

    constexpr Position Clock{ 0, 0 };
    constexpr Position ClockDayOfWeek{ 31, 0 };
    constexpr Position InternalTemperature{ 53, 0 };
    constexpr Position HeatingState{ 81, 0 };

    namespace Zone
    {
        constexpr auto BaseLine{ 2 };
        constexpr std::array Columns{ 0, 64 };

        constexpr auto Name{ 0 };
        constexpr auto Temperature{ 16 };
        constexpr auto SLabel{ 4 };
        constexpr auto Icon{ 40 };
    }
}

MainScreen::MainScreen(Model& model, Graphics& graphics)
    : Screen{ ScreenID::Main, model, graphics }
{}

void MainScreen::activate()
{
    // testMenu.update();
    update();
}

void MainScreen::update()
{
    // testMenu.update();
    drawClock();
    drawInternalTemperature();
    drawHeatingState();
    drawZoneStatuses();
}

Screen::Result MainScreen::handleKeyPress(const Keypad::Keys keys)
{
    using Keys = Keypad::Keys;

    // if (keys & Keys::Plus) {
    //     testMenu.step(UI::StepDirection::Up);
    //     testMenu.update();
    // } else if (keys & Keys::Minus) {
    //     testMenu.step(UI::StepDirection::Down);
    //     testMenu.update();
    // }

    if (keys & Keys::Menu) {
        // Avoid entering the menu while exiting
        // from another screen with long press
        if (!(keys & Keys::LongPress)) {
            return Navigate{ .id = ScreenID::MainMenu };
        }
    }

    return NoAction{};
}

void MainScreen::drawClock() const
{
    char clock[10]{};
    snprintf(clock, sizeof(clock), "%02d:%02d", model().clock.hours, model().clock.minutes);

    graphics().drawText(
        Positions::Clock.x,
        Positions::Clock.line,
        clock,
        Resources::Fonts::Oled
    );

    graphics().drawVerticalSeparator(Positions::ClockDayOfWeek.x, Positions::ClockDayOfWeek.line);

    graphics().drawShortWeekday(
        Positions::ClockDayOfWeek.x + 3,
        Positions::ClockDayOfWeek.line,
        model().clock.dayOfWeek
    );
}

void MainScreen::drawInternalTemperature() const 
{
    char text[9]{};
    snprintf(
        text,
        sizeof(text),
        "%2d.%d",
        model().internalTemperature / 100,
        model().internalTemperature % 100 / 10
    );

    graphics().drawVerticalSeparator(Positions::InternalTemperature.x, Positions::InternalTemperature.line);

    graphics().drawText(
        Positions::InternalTemperature.x + 3,
        Positions::InternalTemperature.line,
        text,
        Resources::Fonts::Oled
    );
}

void MainScreen::drawHeatingState() const
{
    const auto text{
        model().heating ? "Heating"sv : "Idle   "sv
    };

    graphics().drawVerticalSeparator(Positions::HeatingState.x, Positions::HeatingState.line);

    graphics().drawText(
        Positions::HeatingState.x + 3,
        Positions::HeatingState.line,
        text,
        Resources::Fonts::Oled
    );
}

void MainScreen::drawZoneStatus(
    const unsigned line,
    const unsigned column,
    const Model::Zone& zoneModel
) const
{
    if (line > 7 || column > Positions::Zone::Columns.size()) {
        return;
    }

    const auto left{ Positions::Zone::Columns[column] };

    char buf[14]{};

    const auto formatTemperatureIntoBuf = [&](const int temperature) {
        snprintf(
            buf,
            sizeof(buf),
            "%2d.%d",
            temperature / 10,
            temperature % 10
        );
    };

    snprintf(buf, sizeof(buf), "%02d", zoneModel.zoneNumber);
    graphics().drawText(
        left + Positions::Zone::Name,
        line,
        buf,
        Resources::Fonts::Oled
    );

    formatTemperatureIntoBuf(zoneModel.currentTemperature);
    graphics().drawText(
        left + Positions::Zone::Temperature,
        line,
        buf,
        Resources::Fonts::Oled
    );

    graphics().drawText(
        left + Positions::Zone::SLabel,
        line + 1,
        "S:",
        Resources::Fonts::Oled
    );

    if (zoneModel.targetTemperature.has_value()) {
        formatTemperatureIntoBuf(*zoneModel.targetTemperature);
    }
    graphics().drawText(
        left + Positions::Zone::Temperature,
        line + 1,
        zoneModel.targetTemperature.has_value() ? buf : "--.-",
        Resources::Fonts::Oled
    );

    // TODO create bitmaps (2-page) of status icons

    // FIXME until the bitmaps are ready, show the state as text
    graphics().drawText(
        left + Positions::Zone::Icon + 2,
        line,
        [&] {
            switch (zoneModel.status) {
                case Model::Zone::Status::Off:
                    return "OFF"sv;
                case Model::Zone::Status::Idle:
                    return "IDL"sv;
                case Model::Zone::Status::Heating:
                    return "HEA"sv;
                case Model::Zone::Status::Holiday:
                    return "HOL"sv;
                case Model::Zone::Status::Boost:
                    return "BST"sv;
                case Model::Zone::Status::WindowOpen:
                    return "WND"sv;
            }
            return "UNK"sv;
        }(),
        Resources::Fonts::Oled
    );
}

void MainScreen::drawZoneStatuses()
{
    unsigned column{};
    unsigned line{ Positions::Zone::BaseLine };

    for (const auto& zone : model().zones) {
        drawZoneStatus(line, column, zone);

        line += 2;
        if (line >= graphics().Lines) {
            ++column;
            line = Positions::Zone::BaseLine;
        }
    }
}
