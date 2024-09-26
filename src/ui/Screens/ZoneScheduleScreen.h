#pragma once

#include "../Screen.h"
#include "../ScreenID.h"
#include "../Types.h"

namespace UI
{
    class ZoneScheduleScreen : public Screen
    {
    public:
        explicit ZoneScheduleScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        unsigned _scheduleBitIndex{};
        unsigned _scheduleDay{};

        void drawScheduleBar();
        void drawSchedulePositionIndicator();
        void drawWeekday();

        void stepSchedulePosition(StepDirection direction);
        void setScheduleBit(bool on);
    };
}