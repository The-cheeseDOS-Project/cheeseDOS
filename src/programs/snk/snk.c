/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdint.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "beep.h"
#include "string.h"
#include "io.h"
#include "stdbool.h"

#define GAME_WIDTH 80
#define GAME_HEIGHT 23
#define MAX_SNAKE_LENGTH (GAME_WIDTH * GAME_HEIGHT)

#define GUIDE_TEXT "Use arrow keys to switch direction | Hold down an arrow key in the same direction for a speed boost | ESC to quit. | "
#define GUIDE_SCROLL_ROW 24  
#define GUIDE_SCROLL_DELAY 100 

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    int direction;
} Snake;

#define DIR_UP 0
#define DIR_DOWN 1
#define DIR_LEFT 2
#define DIR_RIGHT 3

#define SNAKE_HEAD 'O'
#define SNAKE_BODY '#'
#define FOOD '*'
#define EMPTY ' '

static Snake snake;
static Point food;
static int score;
static int game_running;
static Point last_tail;
static int speed_boost = 0;
static uint8_t key_states[256];

static uint32_t rand_seed = 1;
static uint32_t last_scroll_time = 0;
static int scroll_offset = 0;

static uint32_t simple_rand() {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed / 65536) % 32768;
}

static void init_game() {
    snake.length = 3;
    snake.direction = DIR_RIGHT;
    snake.body[0].x = GAME_WIDTH / 2;
    snake.body[0].y = GAME_HEIGHT / 2;
    snake.body[1].x = snake.body[0].x - 1;
    snake.body[1].y = snake.body[0].y;
    snake.body[2].x = snake.body[0].x - 2;
    snake.body[2].y = snake.body[0].y;

    last_tail = snake.body[snake.length - 1];

    score = 0;
    game_running = 1;

    rand_seed = timer_millis();

    food.x = simple_rand() % GAME_WIDTH;
    food.y = simple_rand() % GAME_HEIGHT;
}

static void draw_game() {
    if (last_tail.x >= 0 && last_tail.y >= 0) {
        set_text_color(COLOR_WHITE, COLOR_BLACK);
        vga_set_cursor(last_tail.y + 1, last_tail.x);
        vga_putchar(EMPTY);
    }

    set_text_color(COLOR_GREEN, COLOR_BLACK);
    vga_set_cursor(snake.body[0].y + 1, snake.body[0].x);
    vga_putchar(SNAKE_HEAD);

    if (snake.length > 1) {
        vga_set_cursor(snake.body[1].y + 1, snake.body[1].x);
        vga_putchar(SNAKE_BODY);
    }

    set_text_color(COLOR_RED, COLOR_BLACK);
    vga_set_cursor(food.y + 1, food.x);
    vga_putchar(FOOD);

    set_text_color(COLOR_CYAN, COLOR_BLACK);
    vga_set_cursor(0, 0);
    print("Score: ");
    print_uint(score);

    set_text_color(COLOR_BLACK, COLOR_BLACK);
}

static void place_food() {
    int valid_position;
    do {
        valid_position = 1;
        food.x = simple_rand() % GAME_WIDTH;
        food.y = simple_rand() % GAME_HEIGHT;

        for (int i = 0; i < snake.length; i++) {
            if (snake.body[i].x == food.x && snake.body[i].y == food.y) {
                valid_position = 0;
                break;
            }
        }
    } while (!valid_position);
}

static int check_collision() {
    Point head = snake.body[0];

    if (head.x < 0) head.x = GAME_WIDTH - 1;
    if (head.x >= GAME_WIDTH) head.x = 0;
    if (head.y < 0) head.y = GAME_HEIGHT - 1;
    if (head.y >= GAME_HEIGHT) head.y = 0;

    snake.body[0] = head;

    for (int i = 1; i < snake.length; i++) {
        if (head.x == snake.body[i].x && head.y == snake.body[i].y) {
            return 1;
        }
    }

    return 0;
}

static void move_snake() {
    last_tail = snake.body[snake.length - 1];

    Point new_head = snake.body[0];

    switch (snake.direction) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
    }

    int food_eaten = (new_head.x == food.x && new_head.y == food.y);

    if (!food_eaten) {
        for (int i = snake.length - 1; i > 0; i--) {
            snake.body[i] = snake.body[i - 1];
        }
    } else {
        for (int i = snake.length; i > 0; i--) {
            snake.body[i] = snake.body[i - 1];
        }
        snake.length++;
        score += 10;
        place_food();

        beep(430, 50);

        last_tail.x = -1;
        last_tail.y = -1;
    }

    snake.body[0] = new_head;
}

static void handle_input() {
    while (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);

        if (scancode & 0x80) {
            key_states[scancode & 0x7F] = 0;
        } else {
            key_states[scancode] = 1;

            switch (scancode) {
                case 0x48:
                    if (snake.direction != DIR_DOWN) snake.direction = DIR_UP;
                    break;
                case 0x50:
                    if (snake.direction != DIR_UP) snake.direction = DIR_DOWN;
                    break;
                case 0x4B:
                    if (snake.direction != DIR_RIGHT) snake.direction = DIR_LEFT;
                    break;
                case 0x4D:
                    if (snake.direction != DIR_LEFT) snake.direction = DIR_RIGHT;
                    break;
                case 0x01:
                    game_running = 0;
                    break;
            }
        }
    }

    speed_boost = 0;
    switch (snake.direction) {
        case DIR_UP:
            if (key_states[0x48]) speed_boost = 1;
            break;
        case DIR_DOWN:
            if (key_states[0x50]) speed_boost = 1;
            break;
        case DIR_LEFT:
            if (key_states[0x4B]) speed_boost = 1;
            break;
        case DIR_RIGHT:
            if (key_states[0x4D]) speed_boost = 1;
            break;
    }
}

static void game_over() {
    set_text_color(COLOR_RED, COLOR_BLACK);
    vga_set_cursor(GAME_HEIGHT / 2, (GAME_WIDTH - 10) / 2);
    print("GAME OVER!");

    vga_set_cursor(GAME_HEIGHT / 2 + 2, (GAME_WIDTH - 15) / 2);
    print("Final Score: ");
    print_uint(score);

    vga_set_cursor(GAME_HEIGHT / 2 + 4, (GAME_WIDTH - 20) / 2);
    print("Press any key to exit");

    beep(60, 100);
    delay(50);
    beep(60, 100);

    keyboard_getchar();
}

void snk(const char* *unused) {
    (void)unused;
    uint8_t old_row, old_col;
    vga_get_cursor(&old_row, &old_col);

    clear_screen();
    vga_hide_cursor(true);

    vga_disable_scroll(true);  

    init_game();

    set_text_color(COLOR_YELLOW, COLOR_BLACK);
    vga_set_cursor(0, (GAME_WIDTH - 3) / 2);
    print("snk");

    set_text_color(COLOR_WHITE, COLOR_BLACK);
    for (int y = 1; y < GAME_HEIGHT; y++) {  
        for (int x = 0; x < GAME_WIDTH; x++) {
            vga_set_cursor(y, x);
            vga_putchar(EMPTY);
        }
    }

    set_text_color(COLOR_GREEN, COLOR_BLACK);
    for (int i = 0; i < snake.length; i++) {
        vga_set_cursor(snake.body[i].y + 1, snake.body[i].x);
        if (i == 0) {
            vga_putchar(SNAKE_HEAD);
        } else {
            vga_putchar(SNAKE_BODY);
        }
    }

    set_text_color(COLOR_RED, COLOR_BLACK);
    vga_set_cursor(food.y + 1, food.x);
    vga_putchar(FOOD);

    uint32_t last_move_time = timer_millis();
    const uint32_t move_delay = 150;
    const uint32_t boost_delay = 75;

    last_scroll_time = timer_millis();
    scroll_offset = 0;

    while (game_running) {
        uint32_t current_time = timer_millis();

        handle_input();

        uint32_t current_delay = speed_boost ? boost_delay : move_delay;
        if (current_time - last_move_time >= current_delay) {
            move_snake();

            if (check_collision()) {
                game_running = 0;
            }

            last_move_time = current_time;
        }

        draw_game();

        if (current_time - last_scroll_time >= GUIDE_SCROLL_DELAY) {
            set_text_color(COLOR_LIGHT_GREY, COLOR_BLACK);

            vga_set_cursor(GUIDE_SCROLL_ROW, 0);
            for (int i = 0; i < GAME_WIDTH; i++) {
                vga_putchar(' ');
            }

            for (int i = 0; i < GAME_WIDTH; i++) {
                char ch = GUIDE_TEXT[(scroll_offset + i) % kstrlen(GUIDE_TEXT)];
                vga_set_cursor(GUIDE_SCROLL_ROW, i);
                vga_putchar(ch);
            }
            scroll_offset = (scroll_offset + 1) % kstrlen(GUIDE_TEXT);
            last_scroll_time = current_time;
        }
    }

    game_over();

    vga_disable_scroll(false); 
    vga_hide_cursor(false);
    vga_set_cursor(old_row, old_col);
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    clear_screen();
}