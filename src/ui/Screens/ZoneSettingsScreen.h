#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class ZoneSettingsScreen : public Screen
    {
    public:
        explicit ZoneSettingsScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu _menu;

        [[nodiscard]] Result selectMenuItem() const;
    };
}