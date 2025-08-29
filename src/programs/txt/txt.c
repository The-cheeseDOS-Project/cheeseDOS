#include "txt.h"

void txt(const char *filename, uint32_t *cwd) {
    if (!filename || *filename == '\0') {
        putstr("Usage: txt <file>\n");
        return;
    }

    char buffer[1024] = {0};
    size_t index = 0;
    bool saved = true;
    bool is_new = false;


    clear_screen();

    ramdisk_inode_t *file = ramdisk_iget_by_name(*cwd, filename);
    if (!file) {
        if (ramdisk_create_file(*cwd, filename) < 0) {
            putstr("Error: Failed to create file.\n");
            return;
        }
        file = ramdisk_iget_by_name(*cwd, filename);
        if (!file) {
            putstr("Error: File creation succeeded, but lookup failed.\n");
            return;
        }
        is_new = true;
    } else {
        memcpy(buffer, file->data, file->size);
        index = file->size;
    }

    set_text_color(COLOR_WHITE, COLOR_BLUE);
    putstr("txt [ESC = Save & Exit | INSERT = Exit (Without save)] ");
    putstr(is_new ? "[NEW FILE] " : "");
    putstr(filename);
    set_text_color(COLOR_WHITE, COLOR_BLACK); 
    
    vga_move_cursor(1, 0);

    for (size_t i = 0; i < index; i++) {
        if (buffer[i] == '\n') {
            uint8_t row, col;
            vga_get_cursor(&row, &col);
            row++;
            col = 0;
            if (row >= get_screen_height()) row = get_screen_height() - 1;
            vga_move_cursor(row, col);
        } else {
            vga_putchar(buffer[i]);
        }
    }

    while (1) {
        int ch = keyboard_getchar();

        if (ch == 27) break; 
        if (ch == KEY_INSERT) {
            saved = false;
            break;
        }

        uint8_t row, col;
        vga_get_cursor(&row, &col);

        if (ch == 8 && index > 0) {
            index--;
            for (size_t i = index; i < kstrlen(buffer); i++) {
                buffer[i] = buffer[i + 1];
            }

            uint8_t redraw_row = row;
            uint8_t redraw_col = col > 0 ? col - 1 : 0;
            if (col == 0 && row > 1) {
                redraw_row--;
                redraw_col = get_screen_width() - 1;
            }
            vga_move_cursor(redraw_row, redraw_col);

            size_t redraw_idx = index;
            uint8_t r = redraw_row, c = redraw_col;
            while (buffer[redraw_idx] != '\0') {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }

            vga_putchar(' ');
            vga_move_cursor(redraw_row, redraw_col);
        }

        else if (ch == KEY_DELETE && index < kstrlen(buffer)) {
            for (size_t i = index; i < kstrlen(buffer); i++) {
                buffer[i] = buffer[i + 1];
            }

            size_t redraw_idx = index;
            uint8_t r = row, c = col;
            while (buffer[redraw_idx] != '\0') {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }

            vga_putchar(' ');
            vga_move_cursor(row, col);
        }

        else if (ch == KEY_ENTER) {
            if (kstrlen(buffer) < sizeof(buffer) - 1) {
                for (size_t i = kstrlen(buffer); i >= index && i < sizeof(buffer) - 1; i--) {
                    buffer[i + 1] = buffer[i];
                }
                buffer[index++] = '\n';
                row++;
                col = 0;
                if (row >= get_screen_height()) row = get_screen_height() - 1;
                vga_move_cursor(row, col);
            }
        }

        else if (ch == KEY_HOME) {

            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }

            size_t target = line_start;
            while (buffer[target] == ' ' && target < kstrlen(buffer)) {
                target++;
            }
            index = target;

            uint8_t row2 = 1;
            for (size_t i = 0; i < line_start; i++) {
                if (buffer[i] == '\n') row2++;
            }
            uint8_t col2 = 0;
            for (size_t i = line_start; i < index; i++) {
                if (buffer[i] != '\n') col2++;
            }
            vga_move_cursor(row2, col2);
        }

        else if (ch == KEY_END) {

            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }

            size_t line_end = line_start;
            while (line_end < kstrlen(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
                line_end++;
            }

            size_t target = line_end;
            while (target > line_start && (buffer[target - 1] == ' ' || buffer[target - 1] == '\t')) {
                target--;
            }

            index = target;

            uint8_t row2 = 1;
            for (size_t i = 0; i < line_start; i++) {
                if (buffer[i] == '\n') row2++;
            }
            uint8_t col2 = 0;
            for (size_t i = line_start; i < index; i++) {
                if (buffer[i] != '\n') col2++;
            }
            vga_move_cursor(row2, col2);
        }

        else if (ch == KEY_LEFT) {
            if (index > 0) {
                index--;
                if (buffer[index] == '\n') {
                    size_t scan = index;
                    uint8_t col2 = 0;
                    while (scan > 0 && buffer[scan - 1] != '\n') {
                        scan--;
                        col2++;
                    }
                    uint8_t row2 = 1;
                    for (size_t i = 0; i < scan; i++) {
                        if (buffer[i] == '\n') row2++;
                    }
                    vga_move_cursor(row2, col2);
                } else {
                    if (col > 0) col--;
                    else if (row > 1) {
                        row--;
                        col = 0;
                        size_t scan = index;
                        while (scan > 0 && buffer[scan - 1] != '\n') {
                            scan--;
                            col++;
                        }
                    }
                    vga_move_cursor(row, col);
                }
            }
        }

        else if (ch == KEY_RIGHT) {
            if (index < kstrlen(buffer)) {
                if (buffer[index] == '\n') {
                    row++;
                    col = 0;
                    if (row >= get_screen_height()) row = get_screen_height() - 1;
                    vga_move_cursor(row, col);
                } else {
                    col++;
                    if (col >= get_screen_width()) {
                        col = 0;
                        row++;
                        if (row >= get_screen_height()) row = get_screen_height() - 1;
                    }
                    vga_move_cursor(row, col);
                }
                index++;
            }
        }

        else if (ch == KEY_UP) {
            size_t curr_start = index;
            while (curr_start > 0 && buffer[curr_start - 1] != '\n') {
                curr_start--;
            }
            if (curr_start > 0) {
                size_t prev_end = curr_start - 1;
                size_t prev_start = prev_end;
                while (prev_start > 0 && buffer[prev_start - 1] != '\n') {
                    prev_start--;
                }
                index = prev_end + 1;
                uint8_t row2 = 1;
                for (size_t i = 0; i < prev_start; i++) {
                    if (buffer[i] == '\n') row2++;
                }
                uint8_t col2 = 0;
                for (size_t i = prev_start; i <= prev_end; i++) {
                    if (buffer[i] != '\n') col2++;
                }
                vga_move_cursor(row2, col2);
            }
        }

        else if (ch == KEY_DOWN) {
            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }
            size_t col_target = index - line_start;
            size_t line_end = index;
            while (line_end < sizeof(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
                line_end++;
            }
            if (buffer[line_end] == '\n') {
                size_t next_start = line_end + 1;
                size_t next_end = next_start;
                while (next_end < sizeof(buffer) && buffer[next_end] != '\n' && buffer[next_end] != '\0') {
                    next_end++;
                }
                size_t next_len = next_end - next_start;
                if (col_target > next_len) col_target = next_len;
                index = next_start + col_target;
                uint8_t row2 = 1;
                for (size_t i = 0; i < index; i++) {
                    if (buffer[i] == '\n') row2++;
                }
                vga_move_cursor(row2, (uint8_t)col_target);
            }
        }

        else if (ch >= 32 && ch <= 126 && kstrlen(buffer) < sizeof(buffer) - 1) {
            for (size_t i = kstrlen(buffer); i >= index && i < sizeof(buffer) - 1; i--) {
                buffer[i + 1] = buffer[i];
            }
            buffer[index++] = (char)ch;

            uint8_t save_row = row, save_col = col;
            size_t redraw_idx = index - 1;
            uint8_t r = row, c = col;
            while (buffer[redraw_idx] != '\0' && r < get_screen_height()) {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }
            vga_move_cursor(save_row, save_col + 1);
        }
    }

    vga_move_cursor(get_screen_height() - 1, 0);
    if (saved) {
        memcpy(file->data, buffer, index);
        file->size = index;
        clear_screen();
        print("Saved as: ");
        print(filename);
        print("\n");
    } else {
        clear_screen();
        print("File was not saved.\n");
    }
}
