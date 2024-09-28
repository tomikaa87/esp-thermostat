#pragma once

#include "../Types.h"

#include <cstdio>
#include <limits>

namespace UI
{
    template <typename CharArray, typename Value>
    void formatTemperatureValue(CharArray& s, const Value value)
    {
        snprintf(
            s,
            sizeof(s),
            "%d.%d C",
            value / 10,
            value % 10
        );
    }

    template <typename CharArray, typename Value>
    void formatValueWithSuffix(CharArray& s, const Value value, const char suffix)
    {
        snprintf(
            s,
            sizeof(s),
            "%d %c",
            value,
            suffix
        );
    }

    template <typename ValueType>
    ValueType stepValue(
        const ValueType value,
        const StepDirection direction,
        const ValueType min = std::numeric_limits<ValueType>::min(),
        const ValueType max = std::numeric_limits<ValueType>::max(),
        const ValueType step = 1
    )
    {
        if (direction == StepDirection::Up) {
            if (value <= (max - step)) {
                return value + step;
            } else {
                return min;
            }
        } else {
            if (value >= (min + step)) {
                return value - step;
            } else {
                return max;
            }
        }
    }
}