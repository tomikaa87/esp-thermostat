#include "ZoneSettingsMenuScreen.h"

using namespace UI;

ZoneSettingsMenuScreen::ZoneSettingsMenuScreen(const Model* model)
    : Screen{ ScreenID::ZoneSettingsMenu, model }
{}

void ZoneSettingsMenuScreen::activate()
{
}

void ZoneSettingsMenuScreen::update()
{
}

Screen::Result ZoneSettingsMenuScreen::handleKeyPress(const Keypad::Keys keys)
{
    return Result{};
}
