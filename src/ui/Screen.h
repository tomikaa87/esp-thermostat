#pragma once

#include "Keypad.h"
#include "ScreenID.h"

#include <cstdint>
#include <concepts>
#include <variant>

namespace UI
{
    class Screen
    {
    public:
        explicit Screen(const int id)
            : _id{ id }
        {}

        [[nodiscard]] int id() const
        {
            return _id;
        }

        struct NoAction {};
        
        struct Navigate {
            int id{ ScreenID::Invalid };
        };

        using Result = std::variant<NoAction, Navigate>;

    private:
        int _id{ ScreenID::Invalid };
    };

    template <typename T>
    concept IsScreen = requires(T s, int id, Keypad::Keys keys) {
        std::derived_from<T, Screen>;
        { s.id() } -> std::same_as<int>;
        { s.handleKeyPress(keys) } -> std::same_as<Screen::Result>;
        { s.activate() } -> std::same_as<void>;
        { s.update() } -> std::same_as<void>;
    };
}