#include "pix.h"



void pix(const char *filename, uint32_t *cwd) {
    if (!filename || *filename == '\0') {
        putstr("Usage: pix <file>");
        return;
    }

    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 80);
    for (volatile int i = 0; i < 2000000; i++);
    while (inb(0x64) & 1) inb(0x60);

    uint16_t* vga_mem = (uint16_t*)0xB8000;
    uint16_t color = ' ' | ((COLOR_BLUE << 4 | COLOR_BLACK) << 8);
    const uint16_t COLORS[] = {
        COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_BROWN, COLOR_YELLOW, COLOR_WHITE,
        COLOR_PURPLE, COLOR_LIGHT_GREY, COLOR_MAGENTA
    };

    int x = 0, y = 1;
    int index = 0;
    
    clear_screen();
    ramdisk_inode_t *file = ramdisk_iget_by_name(*cwd, filename);
    if (file) {
        if (file->data[0] == 0 && file->data[1] == 'P' && file->data[2] == 'X')
            memcpy(vga_mem+80, file->data + 3, 1840 * 2);
    }

    set_text_color(COLOR_WHITE, COLOR_BLUE);
    putstr("pix [ESC = Save & Exit | INSERT = Exit (Without save)] ");
    putstr(filename);
    
    vga_move_cursor(24, 0);
    putstr("WORK IN PROGRESS!");
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    
    vga_move_cursor(1, 0);
    vga_mem[1999] = color;
    while (1) {
        int ch = keyboard_getchar();
        if (ch == 27) {
            if (!file) {
                if (ramdisk_create_file(*cwd, filename) < 0) {
                    putstr("Error: Failed to create file.\n");
                    break;
                }
                file = ramdisk_iget_by_name(*cwd, filename);
                if (!file) {
                    putstr("Error: File creation succeeded, but lookup failed.\n");
                    break;
                }
                
                file->data[0] = 0;
                file->data[1] = 'P';
                file->data[2] = 'X';
            }
            
            memcpy(file->data+3, vga_mem + 80, 1840 * 2);
            file->size = 1840 * 2;
            break;
        }
        else if (ch == KEY_INSERT) {
            break;
        }

        switch (ch) {
          case KEY_LEFT:
              if (x > 0)
                x--;
              break;
          case KEY_RIGHT:
              if (x < 79)
                x++;
              break;
          case KEY_DOWN:
              if (y < 23)
                  y++;
              break;
          case KEY_UP:
              if (y > 1)
                y--;
              break;
          case ' ':
              index = (y * 80) + x;
              if (vga_mem[index] == color)
                  vga_mem[index] = ' ' | ((COLOR_BLACK << 4 | COLOR_WHITE) << 8);
              else
                  vga_mem[index] = color;
              break;
          default:
              if (ch >= '1' && ch <= '9') {
                  color = ' ' | ((COLORS[ch - '0' - 1] << 4 | COLOR_BLACK) << 8);
                  vga_mem[1999] = color;
              }
              break;
        }
    
        vga_move_cursor(y, x);
    }

    while (inb(0x64) & 1) inb(0x60);
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
}
