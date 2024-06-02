#pragma once

#include "PrivateConfig.h"

#include "../Screen.h"
#include "../ScreenID.h"

#include <array>

namespace UI
{
    class MainScreen : public Screen
    {
    public:
        MainScreen(const Model* model);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        void drawClock();
    };
}