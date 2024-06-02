#pragma once

#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class MainMenuScreen : public Screen
    {
    public:
        MainMenuScreen(const Model* model);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);
    };
}