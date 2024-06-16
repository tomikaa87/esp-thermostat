#include "MainScreen.h"


#include "../Menu.h"
#include "../Model.h"

#include <cstdio>

using namespace UI;

namespace {
    Graphics graphics;
    Menu testMenu{
        graphics,
        0,
        5,
        MenuItem{ "Item 1" }
        , MenuItem{ "Item 2" }
        , MenuItem{ "Item 3" }
        , MenuItem{ "Item 4" }
        , MenuItem{ "Item 5" }
        , MenuItem{ "Item 6" }
        , MenuItem{ "Item 7" }
        , MenuItem{ "Item 8" }
        , MenuItem{ "Item 9" }
        , MenuItem{ "Item 10" }
    };
}

MainScreen::MainScreen(const Model* model)
    : Screen{ ScreenID::Main, model }
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
}

Screen::Result MainScreen::handleKeyPress(const Keypad::Keys keys)
{
    // using Keys = Keypad::Keys;

    // if (keys & Keys::Plus) {
    //     testMenu.step(UI::StepDirection::Up);
    //     testMenu.update();
    // } else if (keys & Keys::Minus) {
    //     testMenu.step(UI::StepDirection::Down);
    //     testMenu.update();
    // }

    // if (keys & Keys::Menu) {
    //     // Avoid entering the menu while exiting
    //     // from another screen with long press
    //     if (!(keys & Keys::LongPress)) {
    //         return Navigate{ .id = ScreenID::MainMenu };
    //     }
    // }

    return NoAction{};
}

void MainScreen::drawClock()
{
    char s[10] = { 0 };
    sprintf(s, "%02d:%02d", model()->clock.hours, model()->clock.minutes);
    // graphics.drawText(0, 0, s, 0, false);
    graphics.drawText(0, 0, s, Resources::Fonts::Oled);

    graphics.drawBitmap(0, 1, Resources::Assets::FlameIcon);

    graphics.drawLargeNumber(0, 4, -987654, Resources::Fonts::SevenSegment);
}