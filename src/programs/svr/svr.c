#include "svr.h"

void svr(const char*, uint32_t*) {
    static uint32_t rng_state = 0x12345678;

    #define COLOR_COUNT 6
    static const uint8_t rainbow_bg[COLOR_COUNT] = {
        9, 10, 11, 12, 13, 14
    };

    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 0);
    for (volatile int i = 0; i < 2000000; i++);
    while (inb(0x64) & 1) inb(0x60);

    int width = get_screen_width();
    int height = get_screen_height();
    uint16_t* vga_mem = (uint16_t*)0xB8000;

    rng_state = ((rng_state >> 1) ^ (-(rng_state & 1) & 0xd0000001));
    uint8_t bg_index = (rng_state >> 4) % COLOR_COUNT;
    uint8_t bg = rainbow_bg[bg_index];
    uint8_t attr = (bg << 4) | 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pos = y * width + x;
            vga_mem[pos] = ' ' | (attr << 8);
        }
    }

    int frame_counter = 0;

    while (1) {
        if (++frame_counter >= 100) {
            if (inb(0x64) & 1) {
                inb(0x60);
                break;
            }
            frame_counter = 0;
        }

        for (int i = 0; i < 40; i++) {
            rng_state = ((rng_state >> 1) ^ (-(rng_state & 1) & 0xd0000001));
            int x = (rng_state >> 8) % width;
            int y = (rng_state >> 16) % height;
            int pos = y * width + x;

            uint8_t bg_index = (rng_state >> 4) % COLOR_COUNT;
            uint8_t bg = rainbow_bg[bg_index];
            uint8_t attr = (bg << 4) | 0;

            vga_mem[pos] = ' ' | (attr << 8);
        }

        for (volatile int delay = 0; delay < 10000; delay++);
    }

    while (inb(0x64) & 1) inb(0x60);
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
    return;
}
