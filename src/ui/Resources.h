#pragma once

#include <array>
#include <cstdint>

namespace UI::Resources::Fonts
{
    template <std::size_t CharCount, std::size_t CharWidth>
    struct Font
    {
        static_assert(CharCount > 0 && CharCount < 256);
        static_assert(CharWidth > 0 && CharWidth < 129);
        static constexpr auto charCount{ CharCount };
        static constexpr auto charWidth{ CharWidth };
        const uint8_t glyphs[CharCount][CharWidth]{};
        const uint8_t placeholder[CharWidth]{};
    };

    extern const Font<95, 5> Oled;
}

namespace UI::Resources::Assets
{
    template <std::size_t Width, std::size_t Pages>
    struct MultiPageBitmap
    {
        static_assert(Pages > 0 && Pages < 8);
        static_assert(Width > 0 && Width < 129);
        static constexpr auto width{ Width };
        static constexpr auto pages{ Pages };
        using Bitmap = std::array<uint8_t, Width * Pages>;
        Bitmap bitmap{{}};
    };

    extern const uint8_t ArrowRightIcon[7];
    extern const uint8_t EmptyPositionIndicator[3];
    extern const uint8_t FullPositionIndicator[3];

    extern const MultiPageBitmap<20, 3> FlameIcon;
    extern const MultiPageBitmap<20, 3> StandbyIcon;
    extern const MultiPageBitmap<20, 3> CalendarIcon;
}