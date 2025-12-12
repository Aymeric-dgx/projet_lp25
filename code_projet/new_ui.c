#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#include "header.h"



// Prototypes
void ui(const char main_filename[], int *stop_prog);
void draw_header();
void draw_footpage(); // + prévoir une section pour les msg d'erreur ou lors de la recherche de processus
void proc_kill_management(int pid_to_sign, int signal); // Y compris avec l'affichage des msg d'erreur en "live"




void ui(const char main_filename[], int *stop_prog) {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // getch non bloquant

    // Boucle d'affichage graphique
    int running = 1;
    int pid_selected = 1;
    while(running) {

        // Gestion des touches pressées
        int user_input = getchar();
        switch(user_input) {
            case 'q':
                running = 0; // Stopper tout le programme, y compris les autres thread (à gérer dans le manage)
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
                // F5 : Mettre un processus en pause
                break;
            case KEY_F(6):
                // F6 : Arrêter un processus
                break;
            case KEY_F(7):
                // F7 : Tuer un processus
                break;
            case KEY_F(8):
                // F8 : Redémarrer un processus
                break;
        }

        // Fin gestion des touches pressées

        // Dessin de l'en-tête (PID, PPID, ...) et du bas de page (Avec les touches F1, F2, ...)
    }
}
