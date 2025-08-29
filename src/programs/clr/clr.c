#include "clr.h"

void clr(const char* args, uint32_t *cwd) {
    uint8_t new_fg_color = default_text_fg_color;
    if (!args || kstrcmp(args, "hlp") == 0) {
        set_text_color(COLOR_DARKBLUE, COLOR_BLACK);        print("darkblue\n");
        set_text_color(COLOR_DARKGREEN, COLOR_BLACK);       print("darkgreen\n");
        set_text_color(COLOR_DARKCYAN, COLOR_BLACK);        print("darkcyan\n");
        set_text_color(COLOR_DARKRED, COLOR_BLACK);         print("darkred\n");
        set_text_color(COLOR_MAGENTA, COLOR_BLACK);         print("magenta\n");
        set_text_color(COLOR_BROWN, COLOR_BLACK);           print("brown\n");
        set_text_color(COLOR_LIGHT_GREY, COLOR_BLACK);      print("lightgrey\n");
        set_text_color(COLOR_DARK_GREY, COLOR_BLACK);       print("darkgrey\n");
        set_text_color(COLOR_BLUE, COLOR_BLACK);            print("blue\n");
        set_text_color(COLOR_GREEN, COLOR_BLACK);           print("green\n");
        set_text_color(COLOR_CYAN, COLOR_BLACK);            print("cyan\n");
        set_text_color(COLOR_RED, COLOR_BLACK);             print("red\n");
        set_text_color(COLOR_MAGENTA, COLOR_BLACK);         print("magenta\n");
        set_text_color(COLOR_YELLOW, COLOR_BLACK);          print("yellow\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);           print("white\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        print("\nUsage: clr <color>\n");
        return;
    }
    if (kstrcmp(args, "darkblue") == 0) new_fg_color = COLOR_DARKBLUE;
    else if (kstrcmp(args, "darkgreen") == 0) new_fg_color = COLOR_DARKGREEN;
    else if (kstrcmp(args, "darkcyan") == 0) new_fg_color = COLOR_DARKCYAN;
    else if (kstrcmp(args, "darkred") == 0) new_fg_color = COLOR_DARKRED;
    else if (kstrcmp(args, "magenta") == 0) new_fg_color = COLOR_MAGENTA;
    else if (kstrcmp(args, "brown") == 0) new_fg_color = COLOR_BROWN;
    else if (kstrcmp(args, "lightgrey") == 0) new_fg_color = COLOR_LIGHT_GREY;
    else if (kstrcmp(args, "darkgrey") == 0) new_fg_color = COLOR_DARK_GREY;
    else if (kstrcmp(args, "blue") == 0) new_fg_color = COLOR_BLUE;
    else if (kstrcmp(args, "green") == 0) new_fg_color = COLOR_GREEN;
    else if (kstrcmp(args, "cyan") == 0) new_fg_color = COLOR_CYAN;
    else if (kstrcmp(args, "red") == 0) new_fg_color = COLOR_RED;
    else if (kstrcmp(args, "magenta") == 0) new_fg_color = COLOR_MAGENTA;
    else if (kstrcmp(args, "yellow") == 0) new_fg_color = COLOR_YELLOW;
    else if (kstrcmp(args, "white") == 0) new_fg_color = COLOR_WHITE;
    else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Invalid color. Use 'clr hlp' for options.\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    default_text_fg_color = new_fg_color;
    default_text_bg_color = COLOR_BLACK;
    set_text_color(default_text_fg_color, default_text_bg_color);
    clear_screen();
}
