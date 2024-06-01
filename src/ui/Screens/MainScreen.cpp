#include "MainScreen.h"

#include "../Menu.h"

using namespace UI;

namespace {
    DefaultGraphics graphics;
    Menu testMenu{ graphics, 0, 5, MenuItem{ "Item 1" }, MenuItem{ "Item 2" }, MenuItem{ "Item 3" } };
}

MainScreen::MainScreen()
    : Screen{ ScreenID::Main }
{}

void MainScreen::activate()
{
    testMenu.update();
}

void MainScreen::update()
{
    testMenu.update();
}

Screen::Result MainScreen::handleKeyPress(const Keypad::Keys keys)
{
    using Keys = Keypad::Keys;

    if (keys & Keys::Plus) {
        testMenu.step(UI::StepDirection::Up);
        testMenu.update();
    } else if (keys & Keys::Minus) {
        testMenu.step(UI::StepDirection::Down);
        testMenu.update();
    }

    // if (keys & Keys::Menu) {
    //     // Avoid entering the menu while exiting
    //     // from another screen with long press
    //     if (!(keys & Keys::LongPress)) {
    //         return Navigate{ .id = ScreenID::MainMenu };
    //     }
    // }

    return NoAction{};
}
