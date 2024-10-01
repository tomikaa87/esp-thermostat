#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class GeneralSettingsScreen : public Screen
    {
    public:
        explicit GeneralSettingsScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu<2> _menu;

        [[nodiscard]] Result selectMenuItem() const;
        void stepSelectedSetting(StepDirection direction);

        void updateValueLabels();
    };
}