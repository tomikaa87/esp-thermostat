/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2017-01-07
*/

#pragma once

#include "Keypad.h"
#include "Logger.h"
#include "Screen.h"

#include "Screens/MainScreen.h"
#include "Screens/MainMenuScreen.h"
#include "Screens/ZoneSettingsMenuScreen.h"

#include <ctime>
#include <memory>
#include <variant>

class HeatingController;
class ISystemClock;
class Settings;
class TemperatureSensor;

namespace UI
{
    namespace Detail
    {
        template <typename... ScreenTypes>
        struct ScreenHelper
        {
            static constexpr auto TypeCount = sizeof...(ScreenTypes);
            using Screen = std::variant<ScreenTypes...>;
            using Container = std::array<Screen, TypeCount>;

            static Container constructScreens()
            {
                return Container{
                    {
                        ScreenTypes{}...
                    }
                };
            }
        };
    }

    // Register screens here
    using RegisteredScreens = Detail::ScreenHelper<
        MainScreen
        // MainMenuScreen,
        // ZoneSettingsMenuScreen
    >;

    class UIController
    {
    public:
        UIController(
        );

        void task();

        void update();
        void handleKeyPress(Keypad::Keys keys);

    private:
        Keypad _keypad;
        Logger _log{ "Ui" };
        // std::time_t _lastKeyPressTime = 0;

        RegisteredScreens::Container _screens;
        RegisteredScreens::Screen* _currentScreen{};

        void updateActiveState();
        bool isActive() const;

        [[nodiscard]] bool loadScreen(int id);

        [[nodiscard]] static int getId(RegisteredScreens::Screen& screen);
        static void invokeActivate(RegisteredScreens::Screen& screen);
        static void invokeUpdate(RegisteredScreens::Screen& screen);
        [[nodiscard]] static Screen::Result invokeHandleKeyPress(
            RegisteredScreens::Screen& screen,
            Keypad::Keys keys
        );
    };
}

