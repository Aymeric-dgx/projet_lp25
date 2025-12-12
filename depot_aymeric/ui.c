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
void ui(const char main_filename[], int *stop_prog);
void draw_header();
void draw_footpage(); // + prévoir une section pour les msg d'erreur ou lors de la recherche de processus
void proc_kill_management(int pid_to_sign, int signal); // Y compris avec l'affichage des msg d'erreur en "live". NB : singal = SIGKILL, SIGTERM, ...
int can_signal(int pid); // Si retourne : 1 --> OK, -1 --> Pas l'accès, -2 --> Processus inexistant




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





// Gère l'arret, mise en pause, etc des processus, en plus d'afficher les messages d'erreur directement dans le ncursive
void proc_kill_management(int pid_to_sign, int signal) {
    int proc_good = can_signal(pid);

    if(proc_good == 0) {
        // ERREUR --> Pas les droits, à écrire dans ncursives
        return;
    }

    if(proc_good == -1) {
        // ERREUR --> Processus inexistant, à écrire dans ncursives
        return;
    }

    switch(signal) {
        case SIGSTOP:
            kill(pid, signal);
            // Afficher dans ncursives
            break;
        
        case SIGCONT:
            kill(pid, signal);
            // Afficher dans ncursives
            break;

        case SIGKILL:
            kill(pid, signal);
            // Afficher dans ncursives
            break;
        
        case SIGTERM:
            kill(pid, signal);
            // Afficher dans ncursives
            break;
    }
}



// Vérifie si on peut envoyer un signal au processus
int can_signal(int pid) {
    if (kill(pid, 0) == 0) return 1;        // OK
    if (errno == EPERM) return 0;          // Pas les droits
    if (errno == ESRCH) return -1;         // Processus n'existe pas
    return -2;                             // Autre erreur
}
