#include "MainMenuScreen.h"

using namespace UI;

MainMenuScreen::MainMenuScreen(const Model* model)
    : Screen{ ScreenID::MainMenu, model }
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
