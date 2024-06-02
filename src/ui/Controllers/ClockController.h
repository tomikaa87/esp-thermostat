#pragma once

#include "../Model.h"

#include <ISystemClock.h>

#include <time.h>

namespace UI::Controllers
{
    class ClockController
    {
    public:
        static constexpr auto UpdateIntervalMillis{ 500 };

        explicit ClockController(
            Model::Clock& model,
            const ISystemClock& systemClock
        )
            : _model{ model }
            , _systemClock{ systemClock }
        {}

        void task(const uint32_t deltaMillis)
        {
            _lastUpdateMillis += deltaMillis;

            if (_lastUpdateMillis < UpdateIntervalMillis) {
                return;
            }

            _lastUpdateMillis = 0;

            const time_t localTime{ _systemClock.localTime() };
            const auto* t{ gmtime(&localTime) };

            _model.dayOfWeek = t->tm_wday;
            _model.hours = t->tm_hour;
            _model.minutes = t->tm_min;
        }

    private:
        Model::Clock& _model;
        const ISystemClock& _systemClock;
        uint32_t _lastUpdateMillis{};
    };
}