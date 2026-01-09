#ifndef H_HEADER
#define H_HEADER

#include <pthread.h>

#define SHORT_STR_LENGTH 128
#define MEDIUM_STR_LENGTH 256
#define BIG_STR_LENGTH 512
#define MAX_NB_NETWORK_USER 5


// Structure à utiliser avec le network()
typedef struct {
    int use_remote_config;          // 1 si on utilise -c, 0 sinon
    char remote_config_path[MEDIUM_STR_LENGTH];   // chemin lors du -c pour .config (ou le fichier spécifié par le user)
    char connexion_type[16];        // ssh ou telnet (NB : vérifié que c'est bien ça et pas un char autre [ex : fdsfj au lieu de ssh])
    int port;                       // port à utiliser
    char server_ip[64];             // IP ou DNS de la machine (à récupérer soit du -P, soit à extraire dans le -l --> login@machine)
    char username[MEDIUM_STR_LENGTH];             // nom d'utilisateur (à récupérer soit du -u, ou bien à esxtraire dans le -l --> login@machine)
    char psw[MEDIUM_STR_LENGTH];                  // Mdp pour se connecter à la machine
    char server_name[MEDIUM_STR_LENGTH];          // Nom du serveur distant
} Remote_host_data;



/*
nom_serveur1:adresse_serveur:port:username:password:type_connexion1
nom_serveur2:adresse_serveur:port:username:password:type_connexion2
*/

// Nom des fichiers où seront enregistré les processus (pour le local) + fichier d'erreur
extern const char main_file[];
extern const char buffer_file[];
extern const char errors_file[];

// "Globalisation" des mutex afin que les differentes thread puissent bloquer par exemple être les seuls à modifer le main_file ou le errors_file
extern pthread_mutex_t main_mutex;
extern pthread_mutex_t error_mutex;


// Déclaration des fonctions
void local(const char buffer_file[MEDIUM_STR_LENGTH], const char main_file[MEDIUM_STR_LENGTH]);
void ui(char file_list[][MEDIUM_STR_LENGTH], int file_count, int* p_stop_prog);
void network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH], int* p_nb_network_user, int* p_stop_prog);
void write_error(const char error_msg[], int err);

#endif
