#pragma once

#include "Config.h"

#include "../Model.h"
#include "../Screen.h"
#include "../ScreenID.h"

#include <array>

namespace UI
{
    class MainScreen : public Screen
    {
    public:
        MainScreen(Model& model, Graphics& graphics);

        void activate();
        void update();
        [[nodiscard]] Result handleKeyPress(Keypad::Keys keys);

    private:
        void drawClock() const;
        void drawInternalTemperature() const;
        void drawHeatingState() const;
        void drawZoneStatus(
            unsigned line,
            unsigned column,
            const Model::Zone& zoneModel
        ) const;
        void drawZoneStatuses();
        void drawConnectionStatus();
    };
}