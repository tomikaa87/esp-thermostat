#include "MainMenuScreen.h"

using namespace UI;

MainMenuScreen::MainMenuScreen()
    : Screen{ ScreenID::MainMenu }
{}

void MainMenuScreen::activate()
{
}

void MainMenuScreen::update()
{
}

Screen::Result MainMenuScreen::handleKeyPress(const Keypad::Keys keys)
{
    return Result{};
}
