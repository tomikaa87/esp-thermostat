#pragma once

#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class MainScreen : public Screen
    {
    public:
        MainScreen();

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);
    };
}