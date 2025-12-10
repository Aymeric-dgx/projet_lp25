#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "header.h"



// Création du mutex, permettant de "verrouiller" les threads lors d'opérations critiques, càd l'écriture/lecture dans le main_file
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

// Nom des fichiers où seront enregistré les processus
const char main_file[] = "procs_list.txt";
const char buffer_file[] = "buffer_procs_list.txt"; // A "recopier" dans le main_file grâce à rename (car quasi instantanné)


// Prototypes des fonctions pour les thread
void* local_thread(void* arg);
void* ui_thread(void* arg);


int main(int argc, char* argv[]) {
    pthread_t local_th_id;
    pthread_t ui_th_id;

    // Création du thread qui LOCAL
    if (pthread_create(&local_th_id, NULL, local_thread, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    // Création du thread UI
    if (pthread_create(&ui_th_id, NULL, ui_thread, NULL) != 0) {
        perror("Erreur création thread UI");
        return 1;
    }


    // Attente que les threads se terminent (ce qui n’arrivera jamais sauf interruption)
    pthread_join(local_th_id, NULL);
    pthread_join(ui_th_id, NULL);

    pthread_mutex_destroy(&main_mutex);
    return 0;
}






// Thread qui exécute local()
void* local_thread(void* arg) {
    while(1) {
        local(buffer_file, main_file);
        struct timespec t = { .tv_sec = 1, .tv_nsec = 0 };  
        nanosleep(&t, NULL);
    }
    return NULL;
}

// Thread qui exécute ui()
void* ui_thread(void* arg) {
    ui(main_file);  // ta fonction existante
    return NULL;
}
