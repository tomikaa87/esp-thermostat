#include "Display.h"
#include "Text.h"

#include <cstring>

constexpr auto DefaultCharWidth = 5;
constexpr auto DefaultSpaceWidth = 1;

static const uint8_t DefaultCharset[][DefaultCharWidth] = {
    // [space]
    {
        0, 0, 0, 0, 0
    },
    // !
    {
        0b00000000,
        0b00000000,
        0b01001111,
        0b00000000,
        0b00000000
    },
    // "
    {
        0b00000000,
        0b00000111,
        0b00000000,
        0b00000111,
        0b00000000
    },
    // #
    {
        0b00010100,
        0b01111111,
        0b00010100,
        0b01111111,
        0b00010100
    },
    // $
    {
        0b00100100,
        0b00101010,
        0b01111111,
        0b00101010,
        0b00010010
    },
    // %
    {
        0b00100011,
        0b00010011,
        0b00001000,
        0b01100100,
        0b01100010
    },
    // &
    {
        0b00110110,
        0b01001001,
        0b01010101,
        0b00100010,
        0b01010000
    },
    // '
    {
        0b00000000,
        0b00000101,
        0b00000011,
        0b00000000,
        0b00000000
    },
    // (
    {
        0b00000000,
        0b00011100,
        0b00100010,
        0b01000001,
        0b00000000
    },
    // )
    {
        0b00000000,
        0b01000001,
        0b00100010,
        0b00011100,
        0b00000000
    },
    // *
    {
        0b00010100,
        0b00001000,
        0b00111110,
        0b00001000,
        0b00010100
    },
    // +
    {
        0b00001000,
        0b00001000,
        0b00111110,
        0b00001000,
        0b00001000,
    },
    // ,
    {
        0b00000000,
        0b01010000,
        0b00110000,
        0b00000000,
        0b00000000
    },
    // -
    {
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000
    },
    // .
    {
        0b00000000,
        0b01100000,
        0b01100000,
        0b00000000,
        0b00000000
    },
    // /
    {
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010
    },
    // 0
    {
        0b00111110,
        0b01010001,
        0b01001001,
        0b01000101,
        0b00111110
    },
    // 1
    {
        0b00000000,
        0b01000010,
        0b01111111,
        0b01000000,
        0b00000000
    },
    // 2
    {
        0b01000010,
        0b01100001,
        0b01010001,
        0b01001001,
        0b01000110
    },
    // 3
    {
        0b00100001,
        0b01000001,
        0b01000101,
        0b01001011,
        0b00110001
    },
    // 4
    {
        0b00011000,
        0b00010100,
        0b00010010,
        0b01111111,
        0b00010000
    },
    // 5
    {
        0b00100111,
        0b01000101,
        0b01000101,
        0b01000101,
        0b00111001
    },
    // 6
    {
        0b00111100,
        0b01001010,
        0b01001001,
        0b01001001,
        0b00110000
    },
    // 7
    {
        0b00000001,
        0b01110001,
        0b00001001,
        0b00000101,
        0b00000011
    },
    // 8
    {
        0b00110110,
        0b01001001,
        0b01001001,
        0b01001001,
        0b00110110
    },
    // 9
    {
        0b00000110,
        0b01001001,
        0b01001001,
        0b00101001,
        0b00011110
    },
    // :
    {
        0b00000000,
        0b00110110,
        0b00110110,
        0b00000000,
        0b00000000
    },
    // ;
    {
        0b00000000,
        0b01010110,
        0b00110110,
        0b00000000,
        0b00000000
    },
    // <
    {
        0b00001000,
        0b00010100,
        0b00100010,
        0b01000001,
        0b00000000
    },
    // =
    {
        0b00010100,
        0b00010100,
        0b00010100,
        0b00010100,
        0b00010100
    },
    // >
    {
        0b00000000,
        0b01000001,
        0b00100010,
        0b00010100,
        0b00001000
    },
    // ?
    {
        0b00000010,
        0b00000001,
        0b01010001,
        0b00001001,
        0b00000110
    },
    // @
    {
        0b00110010,
        0b01001001,
        0b01111001,
        0b01000001,
        0b00111110
    },
    // A
    {
        0b01111110,
        0b00010001,
        0b00010001,
        0b00010001,
        0b01111110
    },
    // B
    {
        0b01111111,
        0b01001001,
        0b01001001,
        0b01001001,
        0b00110110
    },
    // C
    {
        0b00111110,
        0b01000001,
        0b01000001,
        0b01000001,
        0b00100010
    },
    // D
    {
        0b01111111,
        0b01000001,
        0b01000001,
        0b00100010,
        0b00011100
    },
    // E
    {
        0b01111111,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01000001
    },
    // F
    {
        0b01111111,
        0b00001001,
        0b00001001,
        0b00001001,
        0b00000001
    },
    // G
    {
        0b00111110,
        0b01000001,
        0b01001001,
        0b01001001,
        0b01111010
    },
    // H
    {
        0b01111111,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01111111
    },
    // I
    {
        0b00000000,
        0b01000001,
        0b01111111,
        0b01000001,
        0b00000000
    },
    // J
    {
        0b00100000,
        0b01000000,
        0b01000001,
        0b00111111,
        0b00000001
    },
    // K
    {
        0b01111111,
        0b00001000,
        0b00010100,
        0b00100010,
        0b01000001
    },
    // L
    {
        0b01111111,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000
    },
    // M
    {
        0b01111111,
        0b00000010,
        0b00001100,
        0b00000010,
        0b01111111
    },
    // N
    {
        0b01111111,
        0b00000100,
        0b00001000,
        0b00010000,
        0b01111111
    },
    // O
    {
        0b00111110,
        0b01000001,
        0b01000001,
        0b01000001,
        0b00111110
    },
    // P
    {
        0b01111111,
        0b00001001,
        0b00001001,
        0b00001001,
        0b00000110
    },
    // Q
    {
        0b00111110,
        0b01000001,
        0b01010001,
        0b00100001,
        0b01011110
    },
    // R
    {
        0b01111111,
        0b00001001,
        0b00011001,
        0b00101001,
        0b01000110
    },
    // S
    {
        0b01000110,
        0b01001001,
        0b01001001,
        0b01001001,
        0b00110001
    },
    // T
    {
        0b00000001,
        0b00000001,
        0b01111111,
        0b00000001,
        0b00000001
    },
    // U
    {
        0b00111111,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111111
    },
    // V
    {
        0b00011111,
        0b00100000,
        0b01000000,
        0b00100000,
        0b00011111
    },
    // W
    {
        0b00111111,
        0b01000000,
        0b00111000,
        0b01000000,
        0b00111111
    },
    // X
    {
        0b01100011,
        0b00010100,
        0b00001000,
        0b00010100,
        0b01100011
    },
    // Y
    {
        0b00000111,
        0b00001000,
        0b01110000,
        0b00001000,
        0b00000111
    },
    // Z
    {
        0b01100001,
        0b01010001,
        0b01001001,
        0b01000101,
        0b01000011
    },
    // [
    {
        0b00000000,
        0b01111111,
        0b01000001,
        0b01000001,
        0b00000000
    },
    // backslash
    {
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000
    },
    // ]
    {
        0b00000000,
        0b01000001,
        0b01000001,
        0b01111111,
        0b00000000
    },
    // ^
    {
        0b00000100,
        0b00000010,
        0b00000001,
        0b00000010,
        0b00000100
    },
    // _
    {
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000
    },
    // `
    {
        0b00000000,
        0b00000001,
        0b00000010,
        0b00000100,
        0b00000000
    },
    // a
    {
        0b00100000,
        0b01010100,
        0b01010100,
        0b01010100,
        0b01111000
    },
    // b
    {
        0b01111111,
        0b01001000,
        0b01000100,
        0b01000100,
        0b00111000
    },
    // c
    {
        0b00111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00100000
    },
    // d
    {
        0b00111000,
        0b01000100,
        0b01000100,
        0b01001000,
        0b01111111
    },
    // e
    {
        0b00111000,
        0b01010100,
        0b01010100,
        0b01010100,
        0b00011000
    },
    // f
    {
        0b00001000,
        0b01111110,
        0b00001001,
        0b00000001,
        0b00000010
    },
    // g
    {
        0b00001100,
        0b01010010,
        0b01010010,
        0b01010010,
        0b00111110
    },
    // h
    {
        0b01111111,
        0b00001000,
        0b00000100,
        0b00000100,
        0b01111000
    },
    // i
    {
        0b00000000,
        0b01000100,
        0b01111101,
        0b01000000,
        0b00000000
    },
    // j
    {
        0b00100000,
        0b01000000,
        0b01000000,
        0b00111101,
        0b00000000
    },
    // k
    {
        0b01111111,
        0b00010000,
        0b00101000,
        0b01000100,
        0b00000000
    },
    // l
    {
        0b00000000,
        0b01000001,
        0b01111111,
        0b01000000,
        0b00000000
    },
    // m
    {
        0b01111100,
        0b00000100,
        0b00011000,
        0b00000100,
        0b01111000
    },
    // n
    {
        0b01111100,
        0b00001000,
        0b00000100,
        0b00000100,
        0b01111000
    },
    // o
    {
        0b00111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111000
    },
    // p
    {
        0b01111100,
        0b00010100,
        0b00010100,
        0b00010100,
        0b00001000
    },
    // q
    {
        0b00001000,
        0b00010100,
        0b00010100,
        0b00011000,
        0b01111100
    },
    // r
    {
        0b01111100,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00001000
    },
    // s
    {
        0b01001000,
        0b01010100,
        0b01010100,
        0b01010100,
        0b00100000
    },
    // t
    {
        0b00000100,
        0b00111111,
        0b01000100,
        0b01000000,
        0b00100000
    },
    // u
    {
        0b00111100,
        0b01000000,
        0b01000000,
        0b00100000,
        0b01111100
    },
    // v
    {
        0b00011100,
        0b00100000,
        0b01000000,
        0b00100000,
        0b00011100
    },
    // w
    {
        0b00111100,
        0b01000000,
        0b00110000,
        0b01000000,
        0b00111100
    },
    // x
    {
        0b01000100,
        0b00101000,
        0b00010000,
        0b00101000,
        0b01000100
    },
    // y
    {
        0b00001100,
        0b01010000,
        0b01010000,
        0b01010000,
        0b00111100
    },
    // z
    {
        0b01000100,
        0b01100100,
        0b01010100,
        0b01001100,
        0b01000100
    },
    // {
    {
        0b00000000,
        0b00001000,
        0b00110110,
        0b01000001,
        0b00000000
    },
    // |
    {
        0b00000000,
        0b00000000,
        0b01111111,
        0b00000000,
        0b00000000
    },
    // }
    {
        0b00000000,
        0b01000001,
        0b00110110,
        0b00001000,
        0b00000000
    },
    // ~
    {
        0b00001000,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00001000
    },
};

static const uint8_t DefaultCharPlaceholder[DefaultCharWidth] = {
    0b01111111,
    0b01010101,
    0b01001001,
    0b01010101,
    0b01111111
};

#define SevenSegCharWidth 12
#define SevenSegCharLines 3
static const uint8_t SevenSegCharset[11][SevenSegCharLines][SevenSegCharWidth] = {
    {
        // 0, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 0, Page 1
        {
            0b11111111,
            0b11110111,
            0b11100011,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11100011,
            0b11110111,
            0b11111111
        },
        // 0, Page 2
        {
            0b00111111,
            0b01011111,
            0b01101111,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // 1, Page 0
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11111000,
            0b11111100,
            0b11111110
        },
        // 1, Page 1
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11100011,
            0b11110111,
            0b11111111
        },
        // 1, Page 2
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00001111,
            0b00011111,
            0b00111111
        }
    },
    {
        // 2, Page 0
        {
            0b00000000,
            0b00000001,
            0b00000011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 2, Page 1
        {
            0b11111000,
            0b11110100,
            0b11101100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011011,
            0b00010111,
            0b00001111
        },
        // 2, Page 2
        {
            0b00111111,
            0b01011111,
            0b01101111,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01100000,
            0b01000000,
            0b00000000
        }
    },
    {
        // 3, Page 0
        {
            0b00000000,
            0b00000001,
            0b00000011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 3, Page 1
        {
            0b00000000,
            0b00000000,
            0b00001000,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101011,
            0b11110111,
            0b11111111
        },
        // 3, Page 2
        {
            0b00000000,
            0b01000000,
            0b01100000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // 4, Page 0
        {
            0b11111110,
            0b11111100,
            0b11111000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11111000,
            0b11111100,
            0b11111110
        },
        // 4, Page 1
        {
            0b00001111,
            0b00010111,
            0b00011011,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101011,
            0b11110111,
            0b11111111
        },
        // 4, Page 2
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00001111,
            0b00011111,
            0b00111111
        }
    },
    {
        // 5, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000011,
            0b00000001,
            0b00000000
        },
        // 5, Page 1
        {
            0b00001111,
            0b00010111,
            0b00011011,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101100,
            0b11110100,
            0b11111000
        },
        // 5, Page 2
        {
            0b00000000,
            0b01000000,
            0b01100000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // 6, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000011,
            0b00000001,
            0b00000000
        },
        // 6, Page 1
        {
            0b11111111,
            0b11110111,
            0b11101011,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101100,
            0b11110100,
            0b11111000
        },
        // 6, Page 2
        {
            0b00111111,
            0b01011111,
            0b01101111,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // 7, Page 0
        {
            0b00000000,
            0b00000001,
            0b00000011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 7, Page 1
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11100011,
            0b11110111,
            0b11111111
        },
        // 7, Page 2
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00001111,
            0b00011111,
            0b00111111
        }
    },
    {
        // 8, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 8, Page 1
        {
            0b11111111,
            0b11110111,
            0b11101011,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101011,
            0b11110111,
            0b11111111
        },
        // 8, Page 2
        {
            0b00111111,
            0b01011111,
            0b01101111,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // 9, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b11111011,
            0b11111101,
            0b11111110
        },
        // 9, Page 1
        {
            0b00001111,
            0b00010111,
            0b00011011,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b00011100,
            0b11101011,
            0b11110111,
            0b11111111
        },
        // 9, Page 2
        {
            0b00000000,
            0b01000000,
            0b01100000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01101111,
            0b01011111,
            0b00111111
        }
    },
    {
        // C, Page 0
        {
            0b11111110,
            0b11111101,
            0b11111011,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000111,
            0b00000011,
            0b00000001,
            0b00000000
        },
        // C, Page 1
        {
            0b11111111,
            0b11110111,
            0b11100011,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000
        },
        // C, Page 2
        {
            0b00111111,
            0b01011111,
            0b01101111,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01110000,
            0b01100000,
            0b01000000,
            0b00000000
        }
    }
};


void drawChar(const char c, const uint8_t yOffset, const bool invert)
{
    const uint8_t* charData;

    // If character is not supported, draw placeholder
    if (c >= sizeof(DefaultCharset)) {
        charData = DefaultCharPlaceholder;
    }
    else {
        // Get data for the next character
        charData = DefaultCharset[c - 32];
    }

    Display::sendData(charData, DefaultCharWidth, yOffset, invert);
}

void Text::draw(const char c, const uint8_t line, const uint8_t x, const uint8_t yOffset, const bool invert)
{
    Display::setLine(line);
    Display::setColumn(x);
    drawChar(c, yOffset, invert);
}

uint8_t Text::draw(const char* s, const uint8_t line, uint8_t x, const uint8_t yOffset, const bool invert)
{
    const auto length = strlen(s);

    Display::setLine(line);

    for (uint8_t i = 0; i < length; ++i) {
        Display::setColumn(x);

        x += DefaultCharWidth + 1;

        drawChar(s[i], yOffset, invert);

        // Stop if the next character won't fit
        if (x > Display::Driver::Width - 1)
            return x;

        // Fill the background between letters
        if (i < length - 1) {
            const uint8_t pattern[DefaultSpaceWidth] = { 0 };
            for (uint8_t j = 0; j < DefaultSpaceWidth; ++j) {
                Display::sendData(pattern, DefaultSpaceWidth, yOffset, invert);
            }
        }
    }

    return x;
}

void Text::draw7Seg(const char* number, const uint8_t line, uint8_t x)
{
    const auto length = strlen(number);

    if (length == 0 || line > (7 - SevenSegCharLines) || x > 127)
        return;

    for (uint8_t i = 0; i < length; ++i) {
        // Stop if the next character won't fit
        if (x + SevenSegCharWidth + 1 > Display::Driver::Width - 1)
            return;

        const auto c = number[i];

        // Space character only increments the left offset
        if (c != ' ') {
            if (c == '-') {
                Display::setLine(line + 1);
                Display::setColumn(x);

                // Cost-efficient dash symbol
                const uint8_t charData = 0b00011100;
                for (uint8_t j = 2; j < SevenSegCharWidth - 2; ++j)
                    Display::sendData(charData);
            }
            else {
                // Draw the pages of the character
                for (uint8_t page = 0; page < SevenSegCharLines; ++page) {
                    Display::setLine(line + page);
                    Display::setColumn(x);

                    const uint8_t* charData = nullptr;

                    if (c >= '0' && c <= '9')
                        charData = SevenSegCharset[c - '0'][page];
                    else if (c == 'C' || c == 'c')
                        charData = SevenSegCharset[9 + 1][page];

                    Display::sendData(charData, SevenSegCharWidth);
                }
            }
        }
        else {
            for (uint8_t page = 0; page < SevenSegCharLines; ++page) {
                Display::setLine(line + page);
                Display::setColumn(x);

                uint8_t charData[SevenSegCharWidth] = { 0 };

                Display::sendData(charData, SevenSegCharWidth);
            }
        }

        x += SevenSegCharWidth + 2;
    }
}