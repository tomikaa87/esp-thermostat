#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class MainMenuScreen : public Screen
    {
    public:
        explicit MainMenuScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu _menu;

        [[nodiscard]] Result selectMenuItem() const;
    };
}