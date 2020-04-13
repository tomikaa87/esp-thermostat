#pragma once

#include <cstdint>

#include "Keypad.h"

class Screen
{
public:
    explicit Screen(const char* name)
        : _name(name)
    {}

    const char* name() const
    {
        return _name;
    }

    const char* nextScreen() const
    {
        return _nextScreen;
    }

    enum class Action
    {
        NoAction,
        NavigateBack,
        NavigateForward
    };

    Action navigateForward(const char* name)
    {
        _nextScreen = name;
        return Action::NavigateForward;
    }

    virtual ~Screen() = default;

    virtual void activate() = 0;
    virtual void update() = 0;
    virtual Action keyPress(Keypad::Keys keys) = 0;

private:
    const char* const _name;
    const char* _nextScreen = nullptr;
};