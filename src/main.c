#include "fonts.c"
#include <fcntl.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* • → ∘ ∗ ⊙ ◎ ✓ ※ ○ ● */

#define LIKED_ICON " ∗ "
#define NORMAL_ICON "   "

#define TEMP_FILE "/home/adi/.config/st/.temp_font_file"
#define ST_CONF "/home/adi/.config/st/config.h"

#define JUMP 5

int buffered_read(int file_fd, char *buf, int buflen);
Font *choose_font();

#define CTRL(x) ((x) & 0x1F)
#define CHECK_FILE(file, str)                                                  \
    if (file < 0)                                                              \
    {                                                                          \
        perror(str);                                                           \
        return 1;                                                              \
    }

int main(void) /* <<< */
{
    setlocale(LC_ALL, "");

    Font *font = choose_font();
    if (font == NULL)
    { // exit
        clear();
        endwin();
        return 0;
    }
    return 0;

    int st_file_fd = open(ST_CONF, O_RDONLY);
    CHECK_FILE(st_file_fd, "ST Config File")

    int temp_file_fd = open(TEMP_FILE, O_WRONLY | O_CREAT, 0644);
    CHECK_FILE(temp_file_fd, "Temp File")

    char line[256];
    int n;
    int temp_n;

    int found = 0;
    char line_to_be_changed[256];

    while ((n = buffered_read(st_file_fd, line, sizeof(line))) > 0)
    {
        if (found || strncmp(line, "static char *font =", 19) == 0)
        {
            found = 1;
        }
        else if (strncmp(line, "static float cwscale =", 22) == 0)
        {
            temp_n = snprintf(line_to_be_changed, sizeof(line_to_be_changed),
                              "static float cwscale = %.1f;\n",
                              font->letter_spacing);
            write(temp_file_fd, line_to_be_changed, temp_n);
        }
        else
        {
            write(temp_file_fd, line, n);
        }

        if (found && strchr(line, ';') != NULL)
        {
            found = 0;

            temp_n = snprintf(
                line_to_be_changed, sizeof(line_to_be_changed),
                "static char *font = "
                "\"%s:style=%s:size=%d:antialias=true:autohint=true\";\n",
                font->family, font->weight, font->size);

            write(temp_file_fd, line_to_be_changed, temp_n);
        }
    }

    if (rename(TEMP_FILE, ST_CONF) == -1)
    {
        perror("Rename");
    }

    if (system("cd ~/.config/st; sudo make clean install; cd") == -1)
    {
        perror("Make");
    }

    close(temp_file_fd);
    close(st_file_fd);

    return 0;
} /* >>> */

int buffered_read(int file_fd, char *buf, int buflen) /* <<< */
{
    memset(buf, 0, buflen);

    int i = 0;
    char c;

    while (i < buflen - 1)
    {
        int n = read(file_fd, &c, 1);
        if (n < 0)
        {
            return -1;
        }

        if (n == 0)
        {
            break;
        }

        buf[i++] = c;

        if (c == '\n')
        {
            break;
        }
    }

    buf[i] = '\0';

    return i;
} /* >>> */

Font *choose_font() /* <<< */
{
    initscr();
    noecho();
    curs_set(0);

    int max_y = getmaxy(stdscr) - 2; // -2 cuz padding and exclusive of last
    int num_fonts = sizeof(global_fonts) / sizeof(global_fonts[0]);
    int win_offset = 0;

    int idx = 0;
    char c;

    do
    {
        printw("\n");
        for (int i = 0 + win_offset; i < max_y + win_offset; i++)
        {

            if (global_fonts[i].liked)
            {
                printw(LIKED_ICON);
            }
            else
            {
                printw(NORMAL_ICON);
            }

            if (i == idx)
            {
                attron(A_STANDOUT);
            }
            else
            {
                attroff(A_STANDOUT);
            }
            printw("  %s  ", global_fonts[i].family);

            attroff(A_STANDOUT);
            printw("\n");
        }

        c = getch();

        if (c == 'j')
        {
            idx++;
            if (idx == max_y + win_offset)
                win_offset++;
        }
        else if (c == 'k')
        {
            idx--;
            if (idx == win_offset - 1)
                win_offset--;
        }
        else if (c == CTRL('d'))
        {
            idx += JUMP;
            if (idx >= max_y + win_offset)
                win_offset += JUMP;
        }
        else if (c == CTRL('u'))
        {
            idx -= JUMP;
            if (idx <= win_offset - 1)
                win_offset -= JUMP;
        }
        else if (c == 'q')
            return NULL;

        if (idx < 0)
        {
            idx = sizeof(global_fonts) / sizeof(global_fonts[0]) - 1;
            win_offset = num_fonts - max_y;
        }
        else if (idx > sizeof(global_fonts) / sizeof(global_fonts[0]) - 1)
        {
            idx = 0;
            win_offset = 0;
        }

        clear();

    } while (c != '\n');

    endwin();

    return &global_fonts[idx];
} /* >>> */
