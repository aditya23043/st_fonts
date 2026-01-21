#include "fonts.c"
#include <fcntl.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
•
→
∘
∗
⊙
◎
✓
※
○
●
*/

#define LIKED_ICON " ∗ "
#define NORMAL_ICON "   "

#define TEMP_FILE "/home/adi/.config/st/.temp_font_file"
#define ST_CONF "/home/adi/.config/st/config.h"

int buffered_read(int file_fd, char *buf, int buflen);
Font *choose_font();

int main(void)
{
    setlocale(LC_ALL, "");

    Font *font = choose_font();
    if (font == NULL)
    { // exit
        clear();
        endwin();
        return 0;
    }

    int st_file_fd = open(ST_CONF, O_RDONLY);
    if (st_file_fd < 0)
    {
        perror("ST Config file");
        return 1;
    }

    int temp_file_fd = open(TEMP_FILE, O_WRONLY | O_CREAT, 0644);
    if (temp_file_fd < 0)
    {
        perror("TEMP file");
        return 1;
    }

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
}

int buffered_read(int file_fd, char *buf, int buflen)
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
}

Font *choose_font()
{
    initscr();
    noecho();
    curs_set(0);

    int idx = 0;
    char c;

    do
    {

        printw("\n");
        for (int i = 0; i < sizeof(global_fonts) / sizeof(global_fonts[0]); i++)
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
            idx++;
        else if (c == 'k')
            idx--;
        else if (c == 'q')
            return NULL;

        if (idx < 0)
        {
            idx = sizeof(global_fonts) / sizeof(global_fonts[0]) - 1;
        }
        else if (idx > sizeof(global_fonts) / sizeof(global_fonts[0]) - 1)
        {
            idx = 0;
        }

        clear();

    } while (c != '\n');

    endwin();

    return &global_fonts[idx];
}
