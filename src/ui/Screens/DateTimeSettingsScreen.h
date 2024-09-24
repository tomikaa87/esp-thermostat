#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class DateTimeSettingsScreen : public Screen
    {
    public:
        explicit DateTimeSettingsScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu _menu;

        [[nodiscard]] Result selectMenuItem() const;
    };
}