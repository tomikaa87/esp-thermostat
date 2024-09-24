#pragma once

#include "Graphics.h"
#include "Keypad.h"
#include "ScreenID.h"

#include <cstdint>
#include <concepts>
#include <variant>

namespace UI
{
    struct Model;

    class Screen
    {
    public:
        explicit Screen(const int id, Model& model, Graphics& graphics)
            : _id{ id }
            , _model{ model }
            , _graphics{ graphics }
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

    protected:
        Model& model() const
        {
            return _model;
        }

        Graphics& graphics() const
        {
            return _graphics;
        }

    private:
        int _id{ ScreenID::Invalid };
        Model& _model;
        Graphics& _graphics;
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