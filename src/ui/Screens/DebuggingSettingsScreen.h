#pragma once

#include "../Menu.h"
#include "../Screen.h"
#include "../ScreenID.h"

namespace UI
{
    class DebuggingSettingsScreen : public Screen
    {
    public:
        explicit DebuggingSettingsScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        Menu<4> _menu;

        [[nodiscard]] Result selectMenuItem() const;
        void stepSelectedSetting(StepDirection direction);

        void updateValueLabels();
    };
}