#ifndef H_HEADER
#define H_HEADER

#include <pthread.h>

// Nom des fichiers où seront enregistré les processus
extern const char main_file[];
extern const char buffer_file[];

// "Globalisation" du mutex afin que les differentes fichiers (càd thread) puissent le bloquer pour pare exemple être les seuls à modifer le main_file
extern pthread_mutex_t main_mutex;


// Déclaration des fonctions
void local(const char* buffer_file, const char* main_file);
void ui(const char* main_file);


#endif
