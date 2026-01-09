#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "header.h"

#define MAX_TABS 10

/* ===================== SIGNAL HANDLING ===================== */

/* Variable to track if window was resized */
int window_resized = 0;

/* Function called when window is resized */
void handle_window_resize(int signal_number) {
    window_resized = 1;
}

/* ===================== UTILITY FUNCTIONS ===================== */

/* Count lines in a string */
int count_lines_in_string(const char *text) {
    int lines = 1;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            lines++;
        }
    }

    return lines;
}

/* Remove spaces, tabs, newlines from start and end of string */
char* clean_string(char *text) {
    char *start;
    char *end;

    if (text == NULL) {
        return text;
    }

    /* Remove spaces from beginning */
    start = text;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }

    if (*start == '\0') {
        text[0] = '\0';
        return text;
    }

    /* Remove spaces from end */
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }

    /* Move cleaned string to beginning */
    if (start != text) {
        char *dest = text;
        while (*start) {
            *dest = *start;
            dest++;
            start++;
        }
        *dest = '\0';
    }

    return text;
}

/* Check if string has exactly 7 colon characters */
int check_seven_colons(const char *text) {
    int colon_count = 0;

    if (text == NULL) {
        return 0;
    }

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == ':') {
            colon_count++;
        }
    }

    return (colon_count == 7);
}

/* Count how many VALID lines are in a file (skip comments and invalid lines) */
int count_lines_in_file(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return 0;
    }

    int line_count = 0;
    char buffer[512];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        /* Clean the line */
        char line_copy[512];
        strcpy(line_copy, buffer);
        clean_string(line_copy);

        /* Skip empty lines and comments */
        if (strlen(line_copy) == 0 || line_copy[0] == '#') {
            continue;
        }

        /* Skip lines with wrong format */
        if (!check_seven_colons(line_copy)) {
            continue;
        }

        line_count++;
    }

    fclose(file);
    return line_count;
}

/* ===================== UI FUNCTIONS ===================== */

/* Show a popup message window */
void show_popup_message(const char *message, const char *pid) {
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    int message_lines = count_lines_in_string(message);
    int popup_height = message_lines + 6;
    int popup_width = 60;

    int popup_y = (screen_height - popup_height) / 2;
    int popup_x = (screen_width - popup_width) / 2;

    WINDOW *popup_window = newwin(popup_height, popup_width, popup_y, popup_x);
    box(popup_window, 0, 0);

    /* Split message by lines and display */
    char message_copy[512];
    strcpy(message_copy, message);

    char *current_line = strtok(message_copy, "\n");
    int line_number = 1;

    while (current_line != NULL) {
        mvwprintw(popup_window, line_number, 2, "%s", current_line);
        line_number++;
        current_line = strtok(NULL, "\n");
    }

    /* Display the PID */
    if (pid != NULL && strlen(pid) > 0) {
        mvwprintw(popup_window, line_number + 1, 2, "Selected PID: %s", pid);
    } else {
        mvwprintw(popup_window, line_number + 1, 2, "No process selected");
    }

    mvwprintw(popup_window, popup_height - 2, 2, "Press any key to continue...");

    wrefresh(popup_window);
    wgetch(popup_window);

    werase(popup_window);
    wrefresh(popup_window);
    delwin(popup_window);

    touchwin(stdscr);
    refresh();
}

// Afficher une popup avec un message personalisé
void classic_popup(char* message) {
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    int message_lines = count_lines_in_string(message);
    int popup_height = message_lines + 6;
    int popup_width = 60;

    int popup_y = (screen_height - popup_height) / 2;
    int popup_x = (screen_width - popup_width) / 2;

    WINDOW *popup_window = newwin(popup_height, popup_width, popup_y, popup_x);
    box(popup_window, 0, 0);

    /* Split message by lines and display */
    char message_copy[512];
    strcpy(message_copy, message);

    char *current_line = strtok(message_copy, "\n");
    int line_number = 1;

    while (current_line != NULL) {
        mvwprintw(popup_window, line_number, 2, "%s", current_line);
        line_number++;
        current_line = strtok(NULL, "\n");
    }

        wrefresh(popup_window);
    wgetch(popup_window);

    werase(popup_window);
    wrefresh(popup_window);
    delwin(popup_window);

    touchwin(stdscr);
    refresh();

}

/* ===================== WINDOW DRAWING FUNCTIONS ===================== */

/* Draw the header window with tabs */
void draw_header_window(WINDOW* header_window, char tab_files[][MEDIUM_STR_LENGTH], int current_tab, int total_tabs) {
    /* Get window dimensions */
    int window_width = getmaxx(header_window);
    int window_height = getmaxy(header_window);

    /* Clear and set background */
    werase(header_window);
    wbkgd(header_window, COLOR_PAIR(1));

    /* Draw at fixed row position (not centered) */
    int row = 1;  /* Fixed row near top */
    int x_position = 1;  /* Start from left edge */

    /* Draw each tab */
    for (int i = 0; i < total_tabs; i++) {
        if (tab_files[i] == NULL) {
            continue;
        }

        /* Prepare tab name without .txt extension */
        char display_name[50];
        strncpy(display_name, tab_files[i], 49);
        display_name[49] = '\0';

        /* Remove .txt extension if present */
        char *dot_position = strstr(display_name, ".txt");
        if (dot_position != NULL) {
            *dot_position = '\0';  /* Cut off at .txt */
        }

        /* Calculate tab name length */
        int name_length = strlen(display_name);

        /* Check if we have space to draw this tab */
        if (x_position + name_length + 3 > window_width - 2) {
            break;  /* Not enough space, stop drawing */
        }

        /* Highlight current tab */
        if (i == current_tab) {
            wattron(header_window, A_REVERSE | COLOR_PAIR(2));
        } else {
            wattron(header_window, COLOR_PAIR(1));
        }

        /* Draw tab name with spaces around it */
        mvwprintw(header_window, row, x_position, " %s ", display_name);

        wattroff(header_window, A_REVERSE | COLOR_PAIR(2));

        /* Move position for next tab */
        x_position += name_length + 2;  /* Name + spaces */

        /* Add separator if not last tab */
        if (i < total_tabs - 1 && x_position < window_width - 3) {
            mvwprintw(header_window, row, x_position, "|");
            x_position += 1;  /* Move past separator */
        }
    }

    box(header_window, 0, 0);
    wrefresh(header_window);
}

/* Draw the footer window */
void draw_footer_window(WINDOW* footer_window) {
    /* Get window dimensions */
    int window_height = getmaxy(footer_window);
    int window_width = getmaxx(footer_window);

    /* Clear and set background */
    werase(footer_window);
    wbkgd(footer_window, COLOR_PAIR(1));

    int center_y = (window_height - 1) / 2;

    char footer_text[] = "F1 - Help | F2/F3 - Change Tab | Arrows - Navigate | Q - Quit";
    int text_x = (window_width - strlen(footer_text)) / 2;

    if (text_x < 2) {
        text_x = 2;
    }

    mvwprintw(footer_window, center_y, text_x, "%s", footer_text);

    box(footer_window, 0, 0);
    wrefresh(footer_window);
}

/* Draw the content window with file data */
void draw_content_window(WINDOW* content_window, const char *filename, int start_line, int highlight_line, char *current_pid) {
    if (content_window == NULL || filename == NULL) {
        return;
    }

    /* Get window dimensions */
    int window_height = getmaxy(content_window);
    int window_width = getmaxx(content_window);
    int inner_width = window_width - 2;

    /* Clear window */
    werase(content_window);
    wbkgd(content_window, COLOR_PAIR(3));

    /* Calculate max lines we can display */
    int max_display_lines = window_height - 4;  /* -4 for header, separator, and borders */

    /* Ensure highlight_line is within bounds */
    if (highlight_line < 0) highlight_line = 0;
    if (highlight_line >= max_display_lines) highlight_line = max_display_lines - 1;

    /* Column widths for the table */
    int column_widths[] = {6, 6, 6, 12, 6, 6, 14, 20};

    /* Adjust column widths if window is too small */
    int total_col_width = 0;
    for (int i = 0; i < 8; i++) {
        total_col_width += column_widths[i];
    }
    total_col_width += 7; /* Spaces between columns */

    if (total_col_width > inner_width) {
        float scale = (float)inner_width / (float)total_col_width;
        for (int i = 0; i < 8; i++) {
            column_widths[i] = (int)(column_widths[i] * scale);
            if (column_widths[i] < 3) {
                column_widths[i] = 3;
            }
        }
    }

    /* Try to open the file */
    pthread_mutex_lock(&main_mutex);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        mvwprintw(content_window, 1, 1, "Error: Cannot open file '%s'", filename);
        box(content_window, 0, 0);
        wrefresh(content_window);
        pthread_mutex_unlock(&main_mutex);
        return;
    }

    /* Draw table header */
    mvwprintw(content_window, 1, 1,
        "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s",
        column_widths[0], "PID",
        column_widths[1], "PPID",
        column_widths[2], "STATE",
        column_widths[3], "USER",
        column_widths[4], "%CPU",
        column_widths[5], "%MEM",
        column_widths[6], "EXEC_TIME",
        column_widths[7], "COMMAND");

    /* Draw separator line */
    for (int i = 1; i < inner_width; i++) {
        mvwaddch(content_window, 2, i, '-');
    }

    /* Read and display file contents */
    char line_buffer[512];
    int lines_skipped = 0;
    int lines_displayed = 0;
    int current_display_line = 3;

    /* Read file line by line */
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL &&
           current_display_line < window_height - 1) {

        /* Clean the line */
        clean_string(line_buffer);

        /* Skip empty lines and comments */
        if (strlen(line_buffer) == 0 || line_buffer[0] == '#') {
            continue;
        }

        /* Check if line has correct format */
        if (!check_seven_colons(line_buffer)) {
            continue;
        }

        /* Skip lines at the beginning (start_line) */
        if (lines_skipped < start_line) {
            lines_skipped++;
            continue;
        }

        /* Stop if we've displayed enough lines */
        if (lines_displayed >= max_display_lines) {
            break;
        }

        /* Split line into fields */
        char fields[8][64];
        int field_count = 0;
        char line_copy[512];
        strcpy(line_copy, line_buffer);
        char *token = strtok(line_copy, ":");

        while (token != NULL && field_count < 8) {
            strncpy(fields[field_count], token, 63);
            clean_string(fields[field_count]);
            token = strtok(NULL, ":");
            field_count++;
        }

        if (field_count != 8) {
            continue;
        }

        /* Format time (seconds to HH:MM:SS) */
        int total_seconds = atoi(fields[6]);
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int seconds = total_seconds % 60;

        char time_string[32];
        sprintf(time_string, "%02d:%02d:%02d", hours, minutes, seconds);

        /* Shorten fields if they are too long */
        for (int i = 0; i < 8; i++) {
            if (strlen(fields[i]) > column_widths[i] - 1) {
                fields[i][column_widths[i] - 3] = '.';
                fields[i][column_widths[i] - 2] = '.';
                fields[i][column_widths[i] - 1] = '\0';
            }
        }

        /* Highlight current line */
        if (lines_displayed == highlight_line) {
            wattron(content_window, A_REVERSE);
            /* Store the PID */
            if (current_pid != NULL) {
                strcpy(current_pid, fields[0]);
            }
        }

        /* Display the line */
        mvwprintw(content_window, current_display_line, 1,
            "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s",
            column_widths[0], fields[0],
            column_widths[1], fields[1],
            column_widths[2], fields[2],
            column_widths[3], fields[3],
            column_widths[4], fields[4],
            column_widths[5], fields[5],
            column_widths[6], time_string,
            column_widths[7], fields[7]);

        if (lines_displayed == highlight_line) {
            wattroff(content_window, A_REVERSE);
        }

        current_display_line++;
        lines_displayed++;
    }

    fclose(file);
    pthread_mutex_unlock(&main_mutex);

    /* Clear any remaining lines */
    for (int i = current_display_line; i < window_height - 1; i++) {
        wmove(content_window, i, 1);
        wclrtoeol(content_window);
    }

    box(content_window, 0, 0);
    wrefresh(content_window);
}

/* Function to completely redraw the interface */
void redraw_interface(WINDOW* header_window, WINDOW* content_window, WINDOW* footer_window,
                      char tab_files[][MEDIUM_STR_LENGTH], int current_tab, int tab_count,
                      int start_line, int highlight_line, char *current_pid) {
    /* Clear stdscr first */
    clear();
    refresh();

    /* Redraw all windows */
    draw_header_window(header_window, tab_files, current_tab, tab_count);
    draw_footer_window(footer_window);

    if (tab_count > 0 && current_tab < tab_count && tab_files[current_tab] != NULL) {
        draw_content_window(content_window, tab_files[current_tab], start_line, highlight_line, current_pid);
    } else {
        werase(content_window);
        mvwprintw(content_window, 1, 1, "No files to display");
        box(content_window, 0, 0);
        wrefresh(content_window);
    }

    refresh();
}




/* ================================= Fonctions for killing procs ============================== */

// Vérifie si on peut envoyer un signal au processus
// Si retourne : 1 --> OK, -1 --> Pas l'accès, -2 --> Processus inexistant
int can_signal(int pid) {
    if (kill(pid, 0) == 0) return 1;        // OK
    if (errno == EPERM) return 0;          // Pas les droits
    if (errno == ESRCH) return -1;         // Processus n'existe pas
    return -2;                             // Autre erreur
}


// Fonction pour tuer le processus
void kill_processus(char* s_proc_pid, int signal) {
    int proc_pid = atoi(s_proc_pid);
    int proc_access = can_signal(proc_pid);

    switch(proc_access) {
        case 1: {
            kill(proc_pid, signal);
            switch(signal) {
                case SIGSTOP: classic_popup("Processus arrêté");
                case SIGKILL: classic_popup("Processus tué");
                case SIGTERM: classic_popup("Processus terminé");
                case SIGCONT: classic_popup("Processus remis en route");
            }
        }
        case 0: classic_popup("Vous n'avez pas les droits pour ce processus");
        case -1: classic_popup("Ce processus n'existe plus");
        case -2: classic_popup("Une erreur est survenu lors du kill de ce processus");
    }
    
}






/* ===================== MAIN UI FUNCTION ===================== */

int start_user_interface(char tab_files[][MEDIUM_STR_LENGTH], int tab_count, int* stop_prog) {
    /* Initialize ncurses */
    initscr();

    /* Setup signal handler for window resize */
    signal(SIGWINCH, handle_window_resize);

    /* Configure ncurses */
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100); /* Non-blocking input with timeout */

    /* Setup colors */
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);   /* Header and footer */
    init_pair(2, COLOR_BLACK, COLOR_WHITE);  /* Active tab */
    init_pair(3, COLOR_WHITE, COLOR_BLACK);  /* Content */

    /* Get initial screen size */
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    /* Create windows */
    WINDOW* header_window = newwin(3, screen_width, 0, 0);
    int content_height = screen_height - 6;
    WINDOW* content_window = newwin(content_height, screen_width, 3, 0);
    WINDOW* footer_window = newwin(3, screen_width, screen_height - 3, 0);


    /* Navigation variables */
    int start_line = 0;
    int highlight_line = 0;
    int line_count = 0;
    int current_tab = 0;
    char current_pid[64] = "";  /* Stocker le PID comme chaîne de caractères */

    /* Get line count for first tab */
    if (tab_count > 0 && tab_files[current_tab] != NULL) {
        line_count = count_lines_in_file(tab_files[current_tab]);
    }

    /* Initial display - draw everything */
    redraw_interface(header_window, content_window, footer_window,
                    tab_files, current_tab, tab_count,
                    start_line, highlight_line, current_pid);

    /* Main loop */
    int program_running = 1;

    while (program_running) {
        /* Check if window was resized */
        if (window_resized) {
            window_resized = 0;

            /* IMPORTANT: Update terminal size information */
            endwin();
            refresh();
            clear();

            /* Reinitialize ncurses */
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(0);
            timeout(100);

            /* Reinitialize colors */
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLUE);
            init_pair(2, COLOR_BLACK, COLOR_WHITE);
            init_pair(3, COLOR_WHITE, COLOR_BLACK);

            /* Get new screen size */
            getmaxyx(stdscr, screen_height, screen_width);

            /* Delete old windows */
            delwin(header_window);
            delwin(content_window);
            delwin(footer_window);

            /* Create new windows with updated size */
            header_window = newwin(3, screen_width, 0, 0);
            content_height = screen_height - 6;
            content_window = newwin(content_height, screen_width, 3, 0);
            footer_window = newwin(3, screen_width, screen_height - 3, 0);

            /* Redraw everything */
            redraw_interface(header_window, content_window, footer_window,
                            tab_files, current_tab, tab_count,
                            start_line, highlight_line, current_pid);
        }

        /* Get user input */
        int user_input = getch();

        if (user_input != ERR) {  /* Only process if we got input */
            switch (user_input) {
                case 'q':
                case 'Q':
                    program_running = 0;
                    *stop_prog = 1;
                    break;

                //Navigate Process
                case KEY_UP:
                    if (highlight_line > 0) {
                        highlight_line--;
                    } else if (start_line > 0) {
                        start_line--;
                    }
                    draw_content_window(content_window, tab_files[current_tab], start_line, highlight_line, current_pid);
                    break;

                case KEY_DOWN:
                    {
                        /* Calculate max display lines in current window */
                        int max_display = content_height - 4;

                        /* Check if we're at the end of the file */
                        if (start_line + highlight_line >= line_count - 1) {
                            /* Already at last line, can't go further */
                            break;
                        }

                        /* Try to move highlight down within visible area */
                        if (highlight_line < max_display - 1) {
                            /* Check there's actually a line to move to */
                            if (start_line + highlight_line + 1 < line_count) {
                                highlight_line++;
                            }
                        }
                        /* Otherwise, scroll down */
                        else if (start_line + max_display < line_count) {
                            start_line++;
                            /* Keep highlight at the same visual position (bottom of screen) */
                        }
                        draw_content_window(content_window, tab_files[current_tab], start_line, highlight_line, current_pid);
                    }
                    break;

                //Manage functions keys
                case KEY_F(1):
                    show_popup_message(
                    "F1 : Help\n"
                    "F2 : Switch to the next tab\n"
                    "F3 : Switch to the previous tab\n"
                    "F4 : Search for a process\n"
                    "F5 : Stop the process\n"
                    "F6 : Terminate the process\n"
                    "F7 : Kill the process\n"
                    "F8 : Resume the process\n"
                    "Q : Quit\n" , current_pid);
                    /* Redraw after popup */
                    redraw_interface(header_window, content_window, footer_window,
                                    tab_files, current_tab, tab_count,
                                    start_line, highlight_line, current_pid);
                    break;

                case KEY_F(2): case KEY_LEFT:
                    if (current_tab > 0) {
                        current_tab--;
                        start_line = 0;
                        highlight_line = 0;
                        if (current_tab < tab_count && tab_files[current_tab] != NULL) {
                            line_count = count_lines_in_file(tab_files[current_tab]);
                        }
                        redraw_interface(header_window, content_window, footer_window,
                                        tab_files, current_tab, tab_count,
                                        start_line, highlight_line, current_pid);
                    }
                break;

                case KEY_F(3): case KEY_RIGHT:
                    if (current_tab < tab_count - 1) {
                        current_tab++;
                        start_line = 0;
                        highlight_line = 0;
                        if (current_tab < tab_count && tab_files[current_tab] != NULL) {
                            line_count = count_lines_in_file(tab_files[current_tab]);
                        }
                        redraw_interface(header_window, content_window, footer_window,
                                        tab_files, current_tab, tab_count,
                                        start_line, highlight_line, current_pid);
                    }
                    break;

                case KEY_F(4): /*search*/ break;
                case KEY_F(5):
                    kill_processus(current_pid, SIGSTOP);
                    redraw_interface(header_window, content_window, footer_window,
                                    tab_files, current_tab, tab_count,
                                    start_line, highlight_line, current_pid);
                    break;
                case KEY_F(6):
                    kill_processus(current_pid, SIGTERM);
                    redraw_interface(header_window, content_window, footer_window,
                                    tab_files, current_tab, tab_count,
                                    start_line, highlight_line, current_pid);
                    break;
                case KEY_F(7):
                    kill_processus(current_pid, SIGKILL);
                    redraw_interface(header_window, content_window, footer_window,
                                    tab_files, current_tab, tab_count,
                                    start_line, highlight_line, current_pid);
                    break;
                case KEY_F(8):
                    kill_processus(current_pid, SIGCONT);
                    redraw_interface(header_window, content_window, footer_window,
                                    tab_files, current_tab, tab_count,
                                    start_line, highlight_line, current_pid);
                    break;
            }
        }
        draw_content_window(content_window, tab_files[current_tab], start_line, highlight_line, current_pid);
    }

    /* Cleanup */
    delwin(header_window);
    delwin(content_window);
    delwin(footer_window);
    endwin();

    return 0;
}

/* ===================== MAIN FUNCTION ===================== */

void ui(char file_list[][MEDIUM_STR_LENGTH], int file_count, int* p_stop_prog) {

    // Ajout du fichier "local" dans la liste des fichiers de données à lire
    char all_files[file_count+1][MEDIUM_STR_LENGTH];
    strcpy(all_files[0], "procs_list.txt");
    for(int i=0 ; i<file_count ; i++) {
        strcpy(all_files[i+1], file_list[i]);
    }

    char msg[512];
    snprintf(msg, sizeof(msg), "Au debut de ui(), tot_nb_user = %d et all_files[0] = %s", file_count, all_files[0]);
    write_error(msg, errno);

    /* Start the user interface */
    start_user_interface(all_files, file_count+1, p_stop_prog); // +1 car on a rajouter le local

}
