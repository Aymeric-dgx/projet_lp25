#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>


#include "header.h"



// Création du mutex, permettant de "verrouiller" les threads lors d'opérations critiques, càd l'écriture/lecture dans le main_file
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex pour l'écriture dans le fichier "errors_file.txt" en cas d'erreurs

// Nom des fichiers où seront enregistré les processus (uniquement en local ?)
const char main_file[] = "procs_list.txt";
const char buffer_file[] = "buffer_procs_list.txt"; // A "recopier" dans le main_file grâce à rename (car quasi instantanné)
const char errors_file[] = "errors_saving.txt"; // Fichier commun à tout les programme et réinitialiser à chaque lancement du .exe (pour éviter les printf qui causerai des bugs à cause des conflit entre les printf et l'afficahge ncursives)


// Prototypes des fonctions pour les thread
void* local_thread(void* arg);
void* ui_thread(void* arg);
void write_error(const char error_msg[]);




int main(int argc, char* argv[]) {
    // "Réinitialisation" du fichier de stockage des erreur
    FILE* errors_saving_file = fopen(errors_file, "w+");
    if(errors_saving_file != NULL) fclose(errors_saving_file);


    pthread_t local_th_id;
    pthread_t ui_th_id;

    // Création du thread qui LOCAL
    if (pthread_create(&local_th_id, NULL, local_thread, NULL) != 0) {
        write_error("Erreur lors de la création du thread de LOCAL");
        return 1;
    }

    // Création du thread UI
    if (pthread_create(&ui_th_id, NULL, ui_thread, NULL) != 0) {
        write_error("Erreur lors de la création du thread de UI");
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
    struct timespec t = { .tv_sec = 1, .tv_nsec = 0 };
    while(1) {
        local(buffer_file, main_file);  
        nanosleep(&t, NULL);
    }
    return NULL;
}

// Thread qui exécute ui()
void* ui_thread(void* arg) {
    ui(main_file);  // ta fonction existante
    return NULL;
}



// Fonction pour écrire dans le fichier "errrors_saving.txt"
void write_error(const char error_msg[]) {
    FILE *file = fopen(errors_file, "a");
    if(file != NULL) {
        fprintf(file, "%s : %s\n", error_msg, strerror(errno));
        fclose(file);
    }
}
