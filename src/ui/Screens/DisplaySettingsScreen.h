#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class DisplaySettingsScreen : public Screen
    {
    public:
        explicit DisplaySettingsScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu _menu;

        [[nodiscard]] Result selectMenuItem() const;
        void stepSelectedSetting(StepDirection direction);

        void updateValueLabels();
    };
}