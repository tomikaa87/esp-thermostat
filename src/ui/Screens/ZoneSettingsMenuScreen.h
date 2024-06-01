#pragma once

#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class ZoneSettingsMenuScreen : public Screen
    {
    public:
        ZoneSettingsMenuScreen();

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);
    };
}