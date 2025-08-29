#include "ban.h"

void ban(const char*, uint32_t *) {
    const uint8_t glyphs[][8] = {
        {0x3C,0x40,0x80,0x80,0x80,0x80,0x40,0x3C},
        {0xFC,0x82,0x81,0x81,0x81,0x81,0x82,0xFC},
        {0x7E,0x81,0x81,0x81,0x81,0x81,0x81,0x7E},
        {0x7C,0x82,0x80,0x7C,0x02,0x82,0x7C,0x00}
    };

    const int count = sizeof(glyphs) / sizeof(glyphs[0]);
    const int gw = 8, gh = 8;
    const int sw = 80, sh = 25;
    uint16_t* vga = (uint16_t*)0xB8000;

    vga_set_cursor(25, 0);

    int x[count], y[count];
    int vx[count], vy[count];
    for (int i = 0; i < count; i++) {
        x[i] = 10 + i * (gw + 2);
        y[i] = 5 + i * 2;
        vx[i] = 1;
        vy[i] = 1;
    }

    uint8_t colors[] = {
        (COLOR_BLUE << 4) | COLOR_WHITE,
        (COLOR_GREEN << 4) | COLOR_WHITE,
        (COLOR_CYAN << 4) | COLOR_WHITE,
        (COLOR_RED << 4) | COLOR_WHITE,
        (COLOR_MAGENTA << 4) | COLOR_WHITE,
        (COLOR_YELLOW << 4) | COLOR_WHITE
    };
    int color_index = 0;

    static const char* banner_top = "The cheese Diskette Operating System | Press ESCAPE (or ESC) key to exit demo. | ";
    static const char* banner_bottom =
        "cheeseDOS is an x86, fully GNU GPLed, custom C99 written, 1.44MB, monolithic, live, Single Address Space Diskette Operating System that loads into RAM. | ";
    static int scroll_top = 0;
    static int scroll_bottom = 0;

    while (1) {
        vga_set_cursor(25, 0);

        if (inb(0x64) & 1 && inb(0x60) == 0x01) break;

        for (int i = 0; i < sw * sh; i++) {
            vga[i] = ' ' | (COLOR_WHITE << 8);
        }

        for (int i = 0; i < sw; i++) {
            char ch = banner_top[(scroll_top + i) % kstrlen(banner_top)];
            vga[i] = ch | ((COLOR_MAGENTA << 4 | COLOR_WHITE) << 8);
        }
        scroll_top = (scroll_top + 1) % kstrlen(banner_top);

        for (int i = 0; i < sw; i++) {
            char ch = banner_bottom[(scroll_bottom + i) % kstrlen(banner_bottom)];
            vga[(sh - 1) * sw + i] = ch | ((COLOR_BLUE << 4 | COLOR_WHITE) << 8);
        }
        scroll_bottom = (scroll_bottom + 1) % kstrlen(banner_bottom);

        for (int i = 0; i < count; i++) {
            x[i] = (x[i] + vx[i] + sw) % sw;
            y[i] += vy[i];

            if (y[i] <= 1 || y[i] + gh >= sh - 1) {
                vy[i] = -vy[i];
                color_index = (color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
            }

            for (int row = 0; row < gh; row++) {
                for (int col = 0; col < gw; col++) {
                    if (glyphs[i][row] & (1 << (7 - col))) {
                        int px = (x[i] + col) % sw;
                        int py = y[i] + row;
                        if (py >= 1 && py < sh - 1) {
                            vga[py * sw + px] = ' ' | (colors[color_index] << 8);
                        }
                    }
                }
            }
        }

        delay(50);
    }

    clear_screen();
}
