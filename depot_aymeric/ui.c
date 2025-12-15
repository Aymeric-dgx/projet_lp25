#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>


#include "header.h"

/* 
NOTE IMPORTANTE
Lorsque tu récupère dans le .txt les données d'un processus, vérifie que la ligne est "complète"
Càd que il n'a pas été stoppé pour x ou y raison et que tu ai une ligne de ce style : 
1958:1445:S:de        (et donc sans tout le rest de la ligne)

PS : C'était à cause de ça que avant de rendre le projet, j'avais parfois des erreurs
*/

// Prototypes
void draw_header();
void draw_procs();
void draw_footpage(); // + prévoir une section pour les msg d'erreur ou lors de la recherche de processus
void proc_kill_management(pid_t pid_to_sign, int signal); // Y compris avec l'affichage des msg d'erreur en "live". NB : singal = SIGKILL, SIGTERM, ...



void ui(const char main_filename[], int *stop_prog) {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // getch non bloquant

    // Boucle d'affichage graphique

    pid_t pid_selected = 1;
    while(!(*stop_prog)) {

        // Gestion des touches pressées
        int user_input = getch();
        switch(user_input) {
            case 'q':
                *stop_prog = 1;
                break;
            case KEY_UP:
                // déplacer la sélection vers le haut
                break;
            case KEY_DOWN:
                // déplacer la sélection vers le bas
                break;
            case KEY_F(1):
                // F1 : Aide
                break;
            case KEY_F(2):
                // F2 : Passer à l’onglet suivant
                break;
            case KEY_F(3):
                // F3 : Passer à l’onglet précédent
                break;
            case KEY_F(4):
                // F4 : Rechercher un processus
                break;
            case KEY_F(5):
                proc_kill_management(pid_selected, SIGSTOP);
                break;
            case KEY_F(6):
                proc_kill_management(pid_selected, SIGTERM);
                break;
            case KEY_F(7):
                proc_kill_management(pid_selected, SIGKILL);
                break;
            case KEY_F(8):
                proc_kill_management(pid_selected, SIGCONT);
                break;
        }
        // Fin gestion des touches pressées

        // Dessin de l'en-tête (PID, PPID, ...) et du bas de page (Avec les touches F1, F2, ...)
    }
}





// Gère l'arret, mise en pause, etc des processus, en plus d'afficher les messages d'erreur directement dans le ncursive
void proc_kill_management(pid_t pid_to_sign, int signal) {
    if(kill(pid_to_sign, signal) == -1) {
        int err = errno;
        write_error("Erreur avec le prog_kill_management() dans ui()", err);
        // + msg dans ncursives (avec errno) ?
        return;
    }
    switch(signal) {
        case SIGSTOP:
            // message dans ncursiece --> Processus mis en pause
            break;
        case SIGCONT:
            // message dans ncursiece --> Processus redémmaré
            break;
        case SIGTERM:
            // message dans ncursiece --> Processus terminé/arrété proprement
            break;
        case SIGKILL:
            // message dans ncursiece --> Processus tué
            break;
    }
    return;
}
