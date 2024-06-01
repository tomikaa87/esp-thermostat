#pragma once

#include <cstdint>

namespace UI::Resources::Fonts::Default
{
    constexpr std::size_t CharacterCount{ 95 };
    constexpr std::size_t CharacterWidth{ 5 };
    extern const uint8_t Data[CharacterCount][CharacterWidth];
    extern const uint8_t PlaceholderData[CharacterWidth];
}

namespace UI::Resources::Assets
{
    extern const uint8_t ArrowRightIcon[7];
    extern const uint8_t EmptyPositionIndicator[3];
    extern const uint8_t FullPositionIndicator[3];
}