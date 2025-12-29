#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <ctype.h>

/* ===================== SIGNAL ===================== */

/* Handle terminal resize */
void handle_resize(int sig) {
    endwin();
    refresh();
    clear();
    resize_term(0, 0);
}

/* ===================== UTILS ===================== */

/* Count the number of lines in a string */
int count_lines_in_str(const char *str) {
    int lines = 1;

    while (*str) {
        if (*str == '\n')
            lines++;
        str++;
    }
    return lines;
}

/* Trim leading and trailing whitespace, '\\n' and '\\r' characters */
char *trim(char *s) {
    char *end;

    // Remove leading spaces
    while (*s && isspace(*s))
        s++;

    if (*s == '\0')
        return s;

    // Remove trailing spaces, \n, \r
    end = s + strlen(s) - 1;
    while (end > s && (isspace(*end) || *end == '\n' || *end == '\r'))
        *end-- = '\0';

    return s;
}


/* Verify if a string contains exactly 7 ':' characters */
int verify_seven_colons(const char *s) {
    int count = 0;

    while (*s) {
        if (*s == ':')
            count++;
        s++;
    }
    return (count == 7);
}

/* Count the number of lines in a file */
int count_lines_in_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f)
        return -1;

    int lines = 0;
    char line[256];

    while (fgets(line, sizeof(line), f) != NULL)
        lines++;

    fclose(f);
    return lines;
}

/* ===================== UI HELPERS ===================== */

/* Display a popup window with a message */
void popup_message(const char *msg) {
    int nb_lines = count_lines_in_str(msg);
    int h = nb_lines + 4;
    int w = 60;

    int y = (LINES - h) / 2;
    int x = (COLS - w) / 2;

    WINDOW *popup = newwin(h, w, y, x);
    box(popup, 0, 0);

    int row = 1;
    const char *start = msg;
    const char *end;

    while ((end = strchr(start, '\n')) != NULL) {
        mvwprintw(popup, row++, 2, "%.*s", (int)(end - start), start);
        start = end + 1;
    }

    mvwprintw(popup, row, 2, "%s", start);
    mvwprintw(popup, h - 2, 2, "Press any key...");

    wgetch(popup);
    werase(popup);    // erase window contents
    wrefresh(popup);  // refresh window to remove it from screen
    delwin(popup);    // delete window

    clear();          // clear the main screen
    refresh();        // refresh to redraw main content

}

/* ===================== UI LAYOUT ===================== */

/* Draw header section */
int draw_header(char *tab_files[], char *glow_one) {
    for (int i = 0; i < 5; i++) {

        int highlight = (strcmp(tab_files[i], glow_one) == 0);

        if (highlight)
            attron(A_REVERSE);

        mvprintw(0, i * 20, "%.*s",
                 (int)strlen(tab_files[i]) - 4,
                 tab_files[i]);

        if (highlight)
            attroff(A_REVERSE);
    }

    mvhline(1, 0, '=', COLS);
    return 2;
}


/* Draw footer section */
int draw_footer(void) {
    mvhline(LINES - 2, 0, '=', COLS);
    mvprintw(LINES - 1, 0, "F1 - Help / F2 F3 F4 F5 F6 F7 F8");
    return 2;
}

/* ===================== DATA ===================== */
/* Read file and display formatted process data */
int draw_data(const char *filename, int header_size, int footer_size, int start_line, int highlight_line) {
    FILE *f = fopen(filename, "r");
    if (!f)
        return 0;

    // Print header
    mvprintw(header_size, 0,
        "%-6s %-6s %-6s %-12s %-6s %-6s %-14s %-20s",
        "PID", "PPID", "STATE", "USER",
        "%CPU", "%MEM", "TPS_EXEC_MMS", "CMD");

    mvhline(header_size + 1, 0, '-', COLS);

    char line[512];

    /* Skip lines before start_line */
    for (int i = 0; i < start_line; i++) {
        if (fgets(line, sizeof(line), f) == NULL) {
            fclose(f);
            return -1;
        }
    }

    int j = header_size;

    while (fgets(line, sizeof line, f)
           && j < LINES - header_size - footer_size) {

        char *s = trim(line);

        if (*s == '\0' || *s == '#')
            continue;

        if (!verify_seven_colons(s)) {
            mvprintw(j + 2, 0, "Invalid process syntax");
            clrtoeol(); // clear rest of line
            j++;
            continue;
        }

        char *tab[8];
        char *opt = strtok(s, ":");

        for (int i = 0; i < 8 && opt != NULL; i++) {
            tab[i] = trim(opt);
            opt = strtok(NULL, ":");
        }

        int time_s = atoi(tab[6]);
        int time_m = 0;
        int time_h = 0;

        while (time_s >= 60) {
            time_m++;
            time_s -= 60;
        }
        while (time_m >= 60) {
            time_h++;
            time_m -= 60;
        }

        char time[32];
        snprintf(time, sizeof(time), "%02d:%02d:%02d", time_h, time_m, time_s);



        if (highlight_line == j-2)
            attron(A_REVERSE);

        mvprintw(j + 2, 0,
    "%-6s %-6s %-6s %-12s %-6s %-6s %-14s %-20s",
    tab[0], tab[1], tab[2], tab[3],
    tab[4], tab[5], time, tab[7]); // fill the rest of the line with spaces

        if (highlight_line == j-2)
            attroff(A_REVERSE);


        clrtoeol(); // clear the rest of the line to remove any leftover characters

        j++;
    }

    fclose(f);
    return 2;  // data_size, same as before
}





/* ===================== MAIN ===================== */



int ui(char** tab_files) {
    initscr();
    signal(SIGWINCH, handle_resize);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    int len = sizeof(tab_files) / sizeof(tab_files[0]);

    int start_line = 0;
    int highlight_line = 0;
    int nb_lines = 0;
    int window = 0;

    int header_size = draw_header(tab_files, tab_files[window]);
    int footer_size = draw_footer();
    int data_size = draw_data(tab_files[0], header_size, footer_size, start_line, highlight_line);
    refresh();
    nb_lines = count_lines_in_file(tab_files[0]);



    int running = 1;
    char* filename = "";


    while (running) {
        switch(window) {
            case 0: {filename = tab_files[0];}
        }
        int user_input = getch();

        switch (user_input) {
            case 'q': running = 0; break;
            case KEY_UP:
                if (start_line > 0) {
                    start_line--;
                    header_size--;
                }

                break;
            case KEY_DOWN:
                if (start_line + LINES - (header_size + data_size + footer_size) < nb_lines)
                    start_line++;
                if (highlight_line + LINES - (header_size + footer_size) < nb_lines)
                    header_size++;
                break;

            case KEY_F(1):
                popup_message(
                    "F1 : Help\n"
                    "F2 : Switch to the next tab\n"
                    "F3 : Switch to the previous tab\n"
                    "F4 : Search for a process\n"
                    "F5 : Stop the process\n"
                    "F6 : Terminate the process\n"
                    "F7 : Kill the process\n"
                    "F8 : Resume the process\n"
                    "Q : Quit\n"
                );
                break;

            case KEY_F(2): break;
            case KEY_F(3): break;
            case KEY_F(4): break;
            case KEY_F(5): break;
            case KEY_F(6): break;
            case KEY_F(7): break;
            case KEY_F(8): break;
        }

        header_size = draw_header(tab_files, tab_files[window]);
        footer_size = draw_footer();
        draw_data(filename, header_size, footer_size, start_line, highlight_line);
        refresh();
    }

    endwin();
    return 0;
}
int main() {
    char* tab_file[] = {"exemple_txt.txt", "", "", "", "", ""};
    ui(tab_file);
}
