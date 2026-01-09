#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/stat.h>

#include "header.h"

// Déclarations de la struture à "remplir" puis à donner à network()
Remote_host_data remote_server;


// Variables pour stocker le nb et le nom des machines externes
int nb_network_user = 0;
char network_users_name[MAX_NB_NETWORK_USER][MEDIUM_STR_LENGTH]; // Au maximum MAX_NB_NETWORK_USER = 5 machines à distance


// Création du mutex, permettant de "verrouiller" les threads lors d'opérations critiques, càd l'écriture/lecture dans le main_file
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex pour l'écriture dans le fichier "errors_file.txt" en cas d'erreurs

// Nom des fichiers où seront enregistré les processus (uniquement en local ?)
const char main_file[MEDIUM_STR_LENGTH] = "procs_list.txt";
const char buffer_file[MEDIUM_STR_LENGTH] = "buffer_procs_list.txt"; // A "recopier" dans le main_file grâce à rename (car quasi instantanné)
const char errors_file[] = "errors_saving.txt"; // Fichier commun à tout les programme et réinitialiser à chaque lancement du .exe (pour éviter les printf qui causerai des bugs à cause des conflit entre les printf et l'afficahge ncursives)

int stop_prog = 0; // Variable "globale" permettant de terminer proprement les threads une fois que l'utilisateur arrete le prog depuis ui()


// Prototypes des fonctions
void* local_thread(void* arg);
void* ui_thread(void* arg);
void* network_thread(void* arg);
void write_error(const char error_msg[], int err);
void print_help(const char *progname);
int dry_run_check_processes(void);
void error_in_opt_network(Remote_host_data remote_server); // Test la validité/cohérence des options lié à network()




int main(int argc, char* argv[]) {

    // "Initialisation" de la structure remote_server
    strcpy(remote_server.connexion_type, "");
    remote_server.port = 0;
    strcpy(remote_server.psw, "");
    strcpy(remote_server.remote_config_path, "");
    strcpy(remote_server.server_ip, "");
    strcpy(remote_server.server_name, "");
    strcpy(remote_server.username, "");
    remote_server.use_remote_config = 0;

    //------------------------------- OPTIONS -------------------------------------//
    struct option options_list[] = {
        {"help",            no_argument,       0, 'h'},
        {"dry-run",         no_argument,       0, 'd'},
        {"remote-config",   optional_argument, 0, 'c'},
        {"connexion-type",  required_argument, 0, 't'},
        {"port",            required_argument, 0, 'P'},
        {"login",           required_argument, 0, 'l'},
        {"remote-server",   required_argument, 0, 's'},
        {"username",        required_argument, 0, 'u'},
        {"password",        required_argument, 0, 'p'},
        {"all",             no_argument,       0, 'a'},
        {0, 0, 0, 0}
    };

    remote_server.port = -1;          // valeur impossible pour savoir si l'utilisateur l'a fourni
    remote_server.use_remote_config = 0;
    strcpy(remote_server.connexion_type, "");
    strcpy(remote_server.server_ip, "");
    strcpy(remote_server.username, "");
    strcpy(remote_server.psw, "");
    strcpy(remote_server.remote_config_path, "");

    //int all_flag = 0;  Inutilisé pour l'instant, servira pour le show_all (pas encore implémenté)
    int network_opts_detected = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "hdc::t:P:l:s:u:p:a", options_list, NULL)) != -1) {
        switch(opt) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);

            case 'd': 
            {
                int ret = dry_run_check_processes();
                exit(ret);
            }

            case 'c':
                network_opts_detected = 1;
                remote_server.use_remote_config = 1;
                if(optarg == NULL) strncpy(remote_server.remote_config_path, ".config", sizeof(remote_server.remote_config_path)); // Si pas d'arg au -c, on utilise le fichier par défaut .config
                else strncpy(remote_server.remote_config_path, optarg, sizeof(remote_server.remote_config_path)); // S'il y'a un arg, on utilise le chmein spécifié par le user
                break;
                
            case 't':
                network_opts_detected = 1;
                strncpy(remote_server.connexion_type, optarg, sizeof(remote_server.connexion_type));
                break;

            case 'P':
                network_opts_detected = 1;
                remote_server.port = atoi(optarg);
                break;

            case 'l':
                network_opts_detected = 1;
                // Si le username ou le server_ip n'a pas déjà été configuré par un -u ou un -p
                if(strlen(remote_server.username) == 0 && strlen(remote_server.server_ip) == 0){
                    // sscanf avec format %[^@] pour tout ce qui est avant le @  (car ^ représente la négation)  
                    if (sscanf(optarg, "%[^@]@%s", remote_server.username, remote_server.server_ip) != 2) {
                        printf("Format de login incorrect\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    printf("Vous avez utilisé -l ET -u + -p. Veuillez n'utiliser que l'une de ses deux possbilitées\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 's':
                network_opts_detected = 1;
                // On vérifie que le server_ip n'a pas déjà été rempli par un -l
                if(strlen(remote_server.server_ip) == 0) strncpy(remote_server.server_ip, optarg, sizeof(remote_server.server_ip));
                else {
                    printf("Vous avez utilisé -l ET -u + -p. Veuillez n'utiliser que l'une de ses deux possbilitées\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'u':
                network_opts_detected = 1;
                // On vérifie que le username n'a pas déjà été rempli par un -l
                if(strlen(remote_server.username) == 0) strncpy(remote_server.username, optarg, sizeof(remote_server.username));
                else {
                    printf("Vous avez utilisé -l ET -u + -p. Veuillez n'utiliser que l'une de ses deux possbilitées\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'p':
                network_opts_detected = 1;
                strncpy(remote_server.psw, optarg, sizeof(remote_server.psw));
                break;

            case 'a':
                // all_flag = 1;  A activer si show_all est implémenté
                break;

            default:
                printf("Option inconnue\n");
                exit(EXIT_FAILURE);
        }
    }
    if(network_opts_detected) error_in_opt_network(remote_server); // Si une erreur est detecté, le programme est stoppé dans cette fonction (avec exit(...))
    //---------------------------------- FIN OPTIONS -----------------------------//



    // "Réinitialisation" du fichier de stockage des erreur
    FILE* errors_saving_file = fopen(errors_file, "w+");
    if(errors_saving_file != NULL) fclose(errors_saving_file);

    

    // Création du thread LOCAL
    pthread_t local_th_id;
    if (pthread_create(&local_th_id, NULL, local_thread, NULL) != 0) {
        write_error("Erreur lors de la création du thread de LOCAL", errno);
        return 1;
    }

    // Création du thread UI
    pthread_t ui_th_id;
    if (pthread_create(&ui_th_id, NULL, ui_thread, NULL) != 0) {
        write_error("Erreur lors de la création du thread de UI", errno);
        return 1;
    }

    // Création du thread NETWORK, et "lancement" de celui ci ssi network est bien activé
    pthread_t network_th_id;
    if(network_opts_detected) {
        if (pthread_create(&network_th_id, NULL, network_thread, NULL) != 0) {
        write_error("Erreur lors de la création du thread de UI", errno);
        return 1;
        }
    }


    // Attente que les threads se terminent (ce qui n’arrivera jamais sauf interruption)
    pthread_join(local_th_id, NULL);
    pthread_join(ui_th_id, NULL);
    if(network_opts_detected) pthread_join(network_th_id, NULL);

    pthread_mutex_destroy(&main_mutex);


    return 0;
}






// Thread qui exécute local()
void* local_thread(void* arg) {
    struct timespec t = { .tv_sec = 1, .tv_nsec = 0 };
    while(!stop_prog) {
        local(buffer_file, main_file);  
        nanosleep(&t, NULL);
    }
    return NULL;
}

// Thread qui exécute ui()
void* ui_thread(void* arg) {
    sleep(5);
    ui(network_users_name, nb_network_user, &stop_prog);
    return NULL;
}

// Thread qui exécute network()
void* network_thread(void* arg) {
    while(!stop_prog) {
        network(remote_server, network_users_name, &nb_network_user, &stop_prog);
    }
    return NULL;
}





// Fonction pour écrire dans le fichier "errrors_saving.txt" une fois qu'on est rentré dans le ncurses
void write_error(const char error_msg[], int err) {
    pthread_mutex_lock(&error_mutex);

    FILE *file = fopen(errors_file, "a");
    if (file != NULL) {
        fprintf(file, "%s : %s\n", error_msg, strerror(err));
        fclose(file);
    }

    pthread_mutex_unlock(&error_mutex);
}



// Affiche l'aide dans le terminal
void print_help(const char *progname) {
    printf("Usage: %s [OPTIONS]\n\n", progname);
    printf("  -h, --help             Affiche cette aide\n");
    printf("  -d, --dry-run          Teste l'accès aux processus locaux/distants sans les afficher\n");
    printf("  -c, --remote-config    Fichier de configuration pour machines distantes\n");
    printf("  -t, --connexion-type   Type de connexion ssh/telnet\n");
    printf("  -P, --port             Port pour la connexion\n");
    printf("  -l, --login            Identifiant et machine distante\n");
    printf("  -s, --remote-server    DNS ou IP de la machine distante\n");
    printf("  -u, --username         Nom de l'utilisateur pour la connexion\n");
    printf("  -p, --password         Mot de passe pour la connexion\n");
    printf("  -a, --all              Collecte locale et distante\n");
}




// Fonction pour tester l'accès aux fichiers tout en restant dans le terminal et sans affichage (EN LOCAL)
int dry_run_check_processes(void) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    int inaccessible_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0]))
            continue;

        pid_t pid = atoi(entry->d_name);
        if (kill(pid, 0) != 0 && errno == ESRCH) {
            inaccessible_count++;
        }
    }

    closedir(dir);

    if (inaccessible_count == 0) {
        printf("Dry-run : tous les processus locaux sont accessibles.\n");
        return 0;
    } else {
        printf("Dry-run : certains processus ne sont pas accessibles.\n");
        return 1;
    }
}









//----- Fonctions vérifiant la conformité / cohérences des options + vérification partielles de l'intégrité des fichiers fournies------//

/*
Potentiel erreur/incohérance :
    (1)-c ET -l, -u, -P, ... utiliser --> Incohérence, pas les deux "types" (mutli_machine ou solo_machine) du network en meme temps
    (2) Le file de -c donné par le user n'existe pas OU n'a pas les bon droits (600 ou rw-------)
    Si -c pas activé :
        (3) - username, psw, server_ip, connexion_type manque --> Erreur "fatale", signaler le manque d'informations
        (4) - connexion_type différent de ssh ou telnet
        (5) - port non spécifié (càd = -1), utiliser le port par defaut du connexion_type (SSH = 22, Telnet = 23)
        (6) - Le fichier de configuration est corrompu --> ligne(s) incomplète(s), ask user s'il autorise la suppression des lignes corrompu
        

NB : 
-l ET -u + -s --> -l est à peu près comme un -u + -s --> L'un ou l'autre, pas les deux DEJA TRAITE dans le getopt
Mauvais forma du login DEJA TRAITE dans le getopt
*/


// Prototype de certaines des fonctions juste en dessous
int check_file_permissions(char *filename);
void check_and_correct_config_file(char *filename);
int is_line_valid(char line[]);




// Fonctions principale, "pilote"
void error_in_opt_network(Remote_host_data rs) {

    if(rs.use_remote_config) {
        // (1)
        if (strlen(rs.connexion_type)>0 || strlen(rs.psw)>0 || strlen(rs.server_ip)>0 || strlen(rs.username)>0) {
            printf("Vous utiliser -c ET -l, -u, -P, ... Veuillez n'utiliser que l'un des deux\n");
            exit(EXIT_FAILURE);
        }

        // (2)
        FILE* remote_config_file = fopen(rs.remote_config_path, "r");
        if(!remote_config_file) {
            printf("Le fichier du -c fourni n'existe pas (ou ne peut pas être ouvert)\n");
            exit(EXIT_FAILURE);
        }
        if(check_file_permissions(rs.remote_config_path) != 1) {
            printf("Le fichier du -c fourni n'a pas les bonnes permissions (ou il y'a eu une erreur lors de la récupération des permissions)\n");
            fclose(remote_config_file);
            exit(EXIT_FAILURE);        
        }
        fclose(remote_config_file);
    }
    

    //(3, 4, 5)
    if(!rs.use_remote_config) {
        // (3)$
        if(strlen(rs.connexion_type)==0 || strlen(rs.psw)==0 || strlen(rs.server_ip)==0 || strlen(rs.username)==0) {
            printf("Vous n'avez pas fourni le username (-u) OU le mdp (-p) OU l'adresse de la machine (-s ou dans -l) OU le type de connexion (-t)\n");
            exit(EXIT_FAILURE);
        }

        // (4)
        if( strcmp(rs.connexion_type, "ssh") != 0 && strcmp(rs.connexion_type, "telnet") != 0 ) {
            printf("Le type de connexion (-t) n'est ni ssh, ni telnet. Veuiller indiquer au moins l'un des deux\n");
            exit(EXIT_FAILURE);
        }

        // (5)
        if(rs.port == -1 && strcmp(rs.connexion_type, "ssh") == 0) rs.port = 22;
        if(rs.port == -1 && strcmp(rs.connexion_type, "telnet") == 0) rs.port = 23;
    }
    
    // (6)
    if(rs.use_remote_config) {
        check_and_correct_config_file(rs.remote_config_path);
    }


}






// Permet de verfier que les permission du file sont bien rw------- (ou 600)
int check_file_permissions(char *filepath) {
    struct stat st;

    if (stat(filepath, &st) != 0) {
        return 0; // erreur lors de la récupération des infos
    }

    // Les droits sur le fichier sont dans st.st_mode (mask S_IRWXU | S_IRWXG | S_IRWXO)
    // On isole les droits avec 0777
    mode_t perms = st.st_mode & 0777;

    if (perms == 0600) {
        return 1; // droits corrects
    } else {
        return -1; // droits incorrects
    }
}




// Dans un premier temps vérifie que toute les lignes de config_file respecte la structure : nom_serveur:adresse_serveur:port:username:password:type_connexion
// Puis, si erreur(s) détécté(s), demande confirmation au user avant de supprimer les lignes corrompu
void check_and_correct_config_file(char* filename) {
    char saved_lines[50][BIG_STR_LENGTH];
    char buffer_line[BIG_STR_LENGTH];
    int nb_lines_saved = 0;
    int error_detected = 0;
    FILE *config_file = fopen(filename, "r");
    if(!config_file) return;

    // On vérifie que le fichier n'est pas vie (puis on reset le pointeur du fichier)
    if(fgets(buffer_line, sizeof(buffer_line), config_file) == NULL) {
        printf("Le fichier que vous avez donné est vide, veuillez indiquer les données de au moins un serveurs, suivant cette structure : \nnom_serveur:adresse_serveur:port:username:password:type_connexion\n");
        fclose(config_file);
        return;
    }
    rewind(config_file);

    // Vérification ligne par ligne du respect ou non de la structure, + sauvegarde des lignes correctes (pour une potentielle réecriture après)
    while(fgets(buffer_line, sizeof(buffer_line), config_file) != NULL) {
        buffer_line[strcspn(buffer_line, "\n")] = '\0';
        if(is_line_valid(buffer_line)) {
            strcpy(saved_lines[nb_lines_saved], buffer_line);
            nb_lines_saved++;
        }
        else {
            printf("Erreure détecté\n");
            error_detected = 1;
        }
    }
    fclose(config_file);

    // Si une erreur a été detecté, on demande l'autorisation à l'utilisateur de corriger le fichier
    if(error_detected) {
        char user_answer;
        printf("Une erreur a été détecté dans le fichier passé en paramètre via -c\nChaque lignes doit respecter cette strucuture --> nom_serveur:adresse_serveur:port:username:password:type_connexion\nAvec 'type_connexion' qui est ssh ou telnet, et le 'port' qui est un nombre\nPeut-on supprimer les lignes corrompu ? [O/N] : ");
        scanf(" %c", &user_answer);
        if(user_answer == 'O' || user_answer == 'o') {
            FILE *new_config_file = fopen(filename, "w+");
            if(!new_config_file) return;
            for(int i=0 ; i<nb_lines_saved ; i++) {
                fprintf(new_config_file, "%s\n", saved_lines[i]);
            }
            printf("Lignes 'corrompus' supprimé\n");
            fclose(new_config_file);
        }
        // Si le user refuse, on termine le programme
        else exit(EXIT_FAILURE);
    }
}




// Retourne 1 ou 0 en fonction de si la ligne respecte la structure ou non
int is_line_valid(char line[]) {
    // Sauvegarde de la line pour éviter les modification sur la line "d'origine" à cause du strtok()
    char* tmp_line = malloc(sizeof(char)*BIG_STR_LENGTH);
    strcpy(tmp_line, line);

    // Premièrement, y'a t'il bien 5 ':' ?
    int i=0;
    int colon_count = 0;
    while(tmp_line[i] != '\0') {
        if(tmp_line[i] == ':') colon_count++;
        i++;
    }
    if(colon_count != 5) {free(tmp_line) ; return 0;}

    // Ensuite, on vérifie que les données sont correctes (au moins en apparance) --> "port" est un nombre, "connexion_type" est ssh ou telnet
    char *token = strtok(tmp_line, ":");
    int nb_field = 1;
    while(token != NULL) {
        if(nb_field == 3) {
            for(int i=0 ; i != '\0' ; i++) {
                if(!isdigit(token[i])) {free(tmp_line) ; return 0;}
            }
        }
        if(nb_field == 6) {
            if( strcmp(token, "ssh") != 0 && strcmp(token, "telnet") != 0) {free(tmp_line) ; return 0;}
        }
        token = strtok(NULL, ":");
        nb_field++;
    }

    // Si tout est okay, alors la ligne est OK
    free(tmp_line);
    
    return 1;
}
