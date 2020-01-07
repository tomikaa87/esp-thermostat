#include "config.h"
#include "ssd1306.h"
#include "sh1106.h"
#include "text.h"
#include "text_input.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

#define TI_ROW_OFFSET           (1)
#define TI_ROW_LEFT_OFFSET      (1)
#define TI_ROW_HEIGHT           (8)
#define TI_CHAR_WIDTH           (5)
#define TI_SPACE_WIDTH          (1)
#define TI_INPUT_FIELD_WIDTH    (21)

#define TI_KEY_MTX_ROWS         (5)
#define TI_KEY_MTX_COLS         (21)

typedef enum {
    SS_OFF,
    SS_ON,
    SS_LOCKED
} shift_state_t;

static struct {
    const char* title;

    struct {
        uint8_t buf;
        uint8_t row;
        uint8_t col;
        uint8_t cursor;
    } pos;

    char* buf;
    uint8_t maxlen;
    uint8_t txt_offset;

    shift_state_t s_shift;
} ctx;

enum {
    K_NONE,
    K_SHIFT,
    K_BCKSPC,
//    K_LEFT,
//    K_RIGHT,
    K_OK,
    K_CANCEL
};

// Repeating key-codes are handled as one large key
static const char key_mtx[TI_KEY_MTX_ROWS][TI_KEY_MTX_COLS] = {
    { '1'    , '2'    , '3'   , '4', '5', '6', '7', '8', '9', '0', '-', '_', '=' , '+', '!', '@'     , '#'     , '$'     , '%'     , '^'     , '&'      },
    { K_NONE , 'q'    , 'w'   , 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']' , '{', '}', '*'     , '('     , ')'     , '~'     , '`'     , K_NONE   },
    { K_SHIFT, K_SHIFT, 'a'   , 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', ':', '"', '\\'    , '|'     , K_BCKSPC, K_BCKSPC, K_BCKSPC, K_BCKSPC },
    { K_SHIFT, K_SHIFT, K_NONE, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '?', '<' , '>', '/'     , K_NONE  , K_BCKSPC, K_BCKSPC, K_BCKSPC, K_BCKSPC },
    { K_OK   , K_OK   , ' '   , ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' , ' ', ' ', K_CANCEL, K_CANCEL, K_CANCEL, K_CANCEL, K_CANCEL, K_CANCEL }
};

static const uint8_t key_shift[2][11] = {
    {
        0b00000000,
        0b10000000,
        0b11000000,
        0b10100000,
        0b10010000,
        0b00001000,
        0b10010000,
        0b10100000,
        0b11000000,
        0b10000000,
        0b00000000
    },
    {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011111,
        0b00010000,
        0b00011111,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000
    }
};

static const uint8_t key_bckspc[2][21] = {
    {
        0b10000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00100010,
        0b01000010,
        0b10000010,
        0b01000010,
        0b00100010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b11111100
    },
    {
        0b00000000,
        0b00000001,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100010,
        0b00100001,
        0b00100000,
        0b00100001,
        0b00100010,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00011111
    }
};

static const uint8_t key_left[] = {
    0b00001000,
    0b00011100,
    0b00101010,
    0b00001000,
    0b00001000
};

static const uint8_t key_right[] = {
    0b00001000,
    0b00001000,
    0b00101010,
    0b00011100,
    0b00001000
};

static const uint8_t key_space[] = {
    0b01100000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01100000
};

typedef enum {
    FC_LEFT,
    FC_RIGHT
} find_col_dir_t;

typedef enum {
    SR_UP,
    SR_DOWN
} sel_row_dir_t;

static void draw();
static void draw_input_field();
static void draw_header();
static void draw_ctrl_keys(int ch);
static void draw_background(
    const uint8_t line,
    const uint8_t start_col,
    const uint8_t width,
    const uint8_t pattern
);
static int find_col(find_col_dir_t dir);
static void select_row(sel_row_dir_t dir);
static ti_key_event_result_t select_key();

static void on_shift_pressed();
static bool is_shifted();
static void on_bckspc_pressed();

static void disp_clear();
static void disp_set_page_addressing();
static void disp_goto_col(uint8_t col);
static void disp_goto_row(uint8_t row);
static void disp_send_data(const uint8_t* data, uint8_t length, uint8_t bit_shift, bool invert);

void text_input_init(char *buf, int buflen, const char* title)
{
    printf("text_input::text_input_init: initializing context...\n");

    memset(&ctx, 0, sizeof(ctx));

    ctx.title = title;

    ctx.buf = buf;
    ctx.maxlen = buflen - 1;

    printf("text_input::text_input_init: clearing text buffer...\n");
    memset(buf, 0, buflen);

    printf("text_input::text_input_init: clearing the display...\n");
    disp_clear();

    printf("text_input::text_input_init: drawing UI elements...\n");
    draw_header();
    draw();
}

ti_key_event_result_t text_input_key_event(ti_key_event_t event)
{
    switch (event) {
    case TI_KE_RIGHT:
        ctx.pos.col = find_col(FC_RIGHT);
        draw();
        break;

    case TI_KE_LEFT:
        ctx.pos.col = find_col(FC_LEFT);
        draw();
        break;

    case TI_KE_DOWN:
        select_row(SR_DOWN);
        draw();
        break;

    case TI_KE_UP:
        select_row(SR_UP);
        draw();
        break;

    case TI_KE_SELECT: {
        const ti_key_event_result_t result = select_key();
        if (result != TI_KE_NO_ACTION)
            return result;
        draw();
    }
    }

    return TI_KE_NO_ACTION;
}

static void draw()
{
#ifdef DEBUG
    printf("text_input::draw: key_mtx size: %ux%u\n", TI_KEY_MTX_COLS, TI_KEY_MTX_ROWS);
#endif

    disp_set_page_addressing();

    for (uint8_t row = 0; row < TI_KEY_MTX_ROWS; ++row) {
        for (uint8_t col = 0; col < TI_KEY_MTX_COLS; ++col) {
            const uint8_t x = col * (TI_CHAR_WIDTH + TI_SPACE_WIDTH) + TI_ROW_LEFT_OFFSET;
            const uint8_t y = row + TI_ROW_OFFSET + 2; /* +2 for the header */
            const char c = key_mtx[row][col];
            const bool highlight = ctx.pos.col == col && ctx.pos.row == row;

#ifdef DEBUG
            printf("text_input::draw(): col=%d, row=%d, x=%d, y=%d\n", col, row, x, y);
#endif

            if (c > ' ') {
#ifdef DEBUG
                printf("text_input::draw(): col=%d, row=%d, x=%d, y=%d, c='%c'\n", col, row, x, y, c);
#endif
                text_draw_char(
                    is_shifted() ? toupper(c) : c,
                    y,
                    x,
                    0,
                    highlight
                );
            }
        }
    }

    const char key = key_mtx[ctx.pos.row][ctx.pos.col];
    draw_ctrl_keys(key);

    draw_input_field();

#ifdef DEBUG
    printf("text_input::draw finished\n");
#endif
}

static void draw_input_field()
{
#ifdef DEBUG
    printf("text_input::draw_input_field: text offset: %d\n", ctx.txt_offset);
#endif

    const uint8_t last_col = text_draw(ctx.buf + ctx.txt_offset, TI_ROW_OFFSET, 1, 0, false);

    // Fill the remaining space
    if (last_col < SSD1306_LCDWIDTH) {
        draw_background(TI_ROW_OFFSET, last_col, SSD1306_LCDWIDTH - last_col, 0);
    }
}

static void draw_header()
{
    printf("text_input::draw_header: drawing title\n");

    text_draw(ctx.title ? ctx.title : "Input text:", 0, 1, 0, false);

    printf("text_input::draw_header: drawing separator\n");

    disp_set_page_addressing();
    disp_goto_row(TI_ROW_OFFSET + 1);
    disp_goto_col(0);

    const uint8_t pattern = 0b00000010;

    for (uint8_t i = 0; i < 128; ++i) {
        disp_send_data(&pattern, 1, 0, false);
    }
}

static void draw_ctrl_keys(int key)
{
    disp_set_page_addressing();

    // Shift
    disp_goto_row(TI_ROW_OFFSET + 4);
    disp_goto_col(1);
    disp_send_data(
        key_shift[0],
        sizeof(key_shift[0]) / sizeof(key_shift[0][0]),
        0,
        key == K_SHIFT
    );
    disp_goto_row(TI_ROW_OFFSET + 5);
    disp_goto_col(1);
    disp_send_data(
        key_shift[1],
        sizeof(key_shift[0]) / sizeof(key_shift[0][0]),
        0,
        key == K_SHIFT
    );

    // Backspace
    disp_goto_row(TI_ROW_OFFSET + 4);
    disp_goto_col(105);
    disp_send_data(
        key_bckspc[0],
        sizeof(key_bckspc[0]) / sizeof(key_bckspc[0][0]),
        0,
        key == K_BCKSPC
    );
    disp_goto_row(TI_ROW_OFFSET + 5);
    disp_goto_col(105);
    disp_send_data(
        key_bckspc[1],
        sizeof(key_bckspc[0]) / sizeof(key_bckspc[0][0]),
        0,
        key == K_BCKSPC
    );

    // OK
    text_draw("OK", TI_ROW_OFFSET + 6, 1, 0, key == K_OK);

    // CANCEL
    text_draw("CANCEL", TI_ROW_OFFSET + 6, 92, 0, key == K_CANCEL);

    // Space
    draw_background(TI_ROW_OFFSET + 6, 13, 36, key == ' ' ? 0xff : 0);
    disp_send_data(key_space, sizeof(key_space) / sizeof(key_space[0]), 0, key == ' ');
    draw_background(TI_ROW_OFFSET + 6, 54, 37, key == ' ' ? 0xff : 0);
}

static void draw_background(
    const uint8_t line,
    const uint8_t start_col,
    const uint8_t width,
    const uint8_t pattern
)
{
    disp_goto_row(line);
    disp_goto_col(start_col);
    for (uint8_t i = 0; i < width; ++i) {
        disp_send_data(&pattern, 1, 0, false);
    }
}

static int find_col(find_col_dir_t dir)
{
#ifdef DEBUG
    printf("text_input::find_col: dir=%s\n", dir == FC_LEFT ? "LEFT" : "RIGHT");
#endif

    if (dir == FC_LEFT && ctx.pos.col <= 0)
        return 0;

    if (dir == FC_RIGHT && ctx.pos.col >= TI_KEY_MTX_COLS - 1)
        return TI_KEY_MTX_COLS - 1;

    char curr_key = key_mtx[ctx.pos.row][ctx.pos.col];
    int next = ctx.pos.col + (dir == FC_RIGHT ? 1 : -1);

#ifdef DEBUG
    printf("text_input::find_col: next candidate: %d\n", next);
#endif

    if (key_mtx[ctx.pos.row][next] != K_NONE && key_mtx[ctx.pos.row][next] != curr_key)
        return next;

#ifdef DEBUG
    printf("text_input::find_col: trying to find a new candidate item\n");
#endif

    while (
        (key_mtx[ctx.pos.row][next] == K_NONE || key_mtx[ctx.pos.row][next] == curr_key)
        && next >= 0
        && next <= TI_KEY_MTX_COLS - 1
    ) {
        next += dir == FC_RIGHT ? 1 : -1;
    }

#ifdef DEBUG
    printf("text_input::find_col: new candidate: %d\n", next);
#endif

    if (next >= 0 && next <= TI_KEY_MTX_COLS - 1)
        return next;

#ifdef DEBUG
    printf("text_input::find_col: returning with original col\n");
#endif

    return ctx.pos.col;
}

static void select_row(sel_row_dir_t dir)
{
    if (
        (dir == SR_UP && ctx.pos.row <= 0)
        || (dir == SR_DOWN && ctx.pos.row >= TI_KEY_MTX_ROWS - 1)
    ) {
        return;
    }

    ctx.pos.row += dir == SR_DOWN ? 1 : -1;

    if (key_mtx[ctx.pos.row][ctx.pos.col] == K_NONE) {
        int next = find_col(FC_RIGHT);
        if (next == ctx.pos.col) {
            next = find_col(FC_LEFT);
            if (next == ctx.pos.col) {
                ctx.pos.row += dir == SR_DOWN ? -1 : 1;
                return;
            }
        }

        ctx.pos.col = next;
    }
}

static ti_key_event_result_t select_key()
{
    const char key = key_mtx[ctx.pos.row][ctx.pos.col];

#ifdef DEBUG
    printf("text_input::select_key: %d\n", key);
#endif

    if (key >= ' ') {
#ifdef DEBUG
        printf("text_input::select_key: selecting printable '%c'\n", key);
#endif

        if (ctx.pos.buf >= ctx.maxlen - 1) {
#ifdef DEBUG
            printf("select_key: text buffer is full\n");
#endif
            return TI_KE_NO_ACTION;
        }

        ctx.buf[ctx.pos.buf] = is_shifted() ? toupper(key) : key;
        ++ctx.pos.buf;

        const int cur_pos = ctx.pos.buf - ctx.txt_offset;
#ifdef DEBUG
        printf("text_input::select_key: cur_pos=%d, bufidx=%d, txt_offset=%d\n",
               cur_pos, ctx.pos.buf, ctx.txt_offset);
#endif

        if (cur_pos > TI_INPUT_FIELD_WIDTH)
            ctx.txt_offset += cur_pos - TI_INPUT_FIELD_WIDTH;
        else if (cur_pos < 0)
            ctx.txt_offset += cur_pos;

#ifdef DEBUG
        printf("text_input::select_key: new buffer index: %d\n", ctx.pos.buf);
#endif

        // Turn of shift if it's not locked
        if (ctx.s_shift == SS_ON)
            ctx.s_shift = SS_OFF;

        draw();
    } else {
        switch (key) {
        case K_SHIFT:
            on_shift_pressed();
            break;
        case K_BCKSPC:
            on_bckspc_pressed();
            break;
        case K_OK:
            disp_clear();
            return TI_KE_ACCEPT;
        case K_CANCEL:
            disp_clear();
            return TI_KE_CANCEL;
        }
    }

    return TI_KE_NO_ACTION;
}

static void on_shift_pressed()
{
#ifdef DEBUG
    printf("text_input::on_shift_pressed: ");
#endif

    switch (ctx.s_shift) {
    case SS_OFF:
#ifdef DEBUG
        printf("OFF->ON\n");
#endif
        ctx.s_shift = SS_ON;
        break;
    case SS_ON:
#ifdef DEBUG
        printf("ON->LOCKED\n");
#endif
        ctx.s_shift = SS_LOCKED;
        break;
    case SS_LOCKED:
#ifdef DEBUG
        printf("LOCKED->OFF\n");
#endif
        ctx.s_shift = SS_OFF;
        break;
    default:
#ifdef DEBUG
        printf("UNKNOWN\n");
#endif
        break;
    }
}

static bool is_shifted()
{
    return ctx.s_shift == SS_ON || ctx.s_shift == SS_LOCKED;
}

static void on_bckspc_pressed()
{
    if (ctx.pos.buf > 0) {
        --ctx.pos.buf;
        ctx.buf[ctx.pos.buf] = 0;

        const int cur_pos = ctx.pos.buf - ctx.txt_offset;
        if (cur_pos < 0)
            ctx.txt_offset += cur_pos;

#ifdef DEBUG
        printf("text_input::on_bckspc_pressed: pos.buf=%d, cur_pos=%d, txt_offset=%d\n",
               ctx.pos.buf, cur_pos, ctx.txt_offset);
#endif
    }
}

static void disp_clear()
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_clear();
#else
    sh1106_clear();
#endif
}

static void disp_set_page_addressing()
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_page_addressing();
#endif
}

static void disp_goto_col(uint8_t col)
{
#ifndef CONFIG_USE_OLED_SH1106		
    ssd1306_set_start_column(col);
#else
    sh1106_set_col_addr(col);
#endif
}

static void disp_goto_row(uint8_t row)
{
#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_set_page(row);
#else
	sh1106_set_page_addr(row);
#endif
}

static void disp_send_data(const uint8_t* data, uint8_t length, uint8_t bit_shift, bool invert)
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_send_data(data, length, bit_shift, invert);
#else
    sh1106_send_data_array(data, length, bit_shift, invert);
#endif    
}