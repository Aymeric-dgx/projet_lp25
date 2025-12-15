#ifndef H_HEADER
#define H_HEADER

#include <pthread.h>

// Nom des fichiers où seront enregistré les processus + fichier d'erreur
extern const char main_file[];
extern const char buffer_file[];
extern const char errors_file[];

// "Globalisation" des mutex afin que les differentes thread puissent bloquer par exemple être les seuls à modifer le main_file ou le errors_file
extern pthread_mutex_t main_mutex;
extern pthread_mutex_t error_mutex;


// Déclaration des fonctions
void local(const char* buffer_file, const char* main_file);
void ui(const char main_filename[], volatile int *stop_prog);
void write_error(const char error_msg[], int err);


#endif
