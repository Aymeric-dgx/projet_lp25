#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>

// Prototypes
void print_help(const char *progname);
int dry_run_check_processes(void);

int main(int argc, char *argv[]) {

    //------------- Options ------------------//
    struct option options_list[] = {
        {"help",            no_argument,       0, 'h'},
        {"dry-run",         no_argument,       0, 'd'},
        {"remote-config",   required_argument, 0, 'c'},
        {"connexion-type",  required_argument, 0, 't'},
        {"port",            required_argument, 0, 'P'},
        {"login",           required_argument, 0, 'l'},
        {"remote-server",   required_argument, 0, 's'},
        {"username",        required_argument, 0, 'u'},
        {"password",        required_argument, 0, 'p'},
        {"all",             no_argument,       0, 'a'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hdc:t:P:l:s:u:p:a", options_list, NULL)) != -1) {
        switch(opt) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);

            case 'd': {
                int ret = dry_run_check_processes();
                exit(ret);
            }

            case 'c':
                // TODO : gérer remote-config
                break;
            case 't':
                // TODO : connexion-type
                break;
            case 'P':
                // TODO : port
                break;
            case 'l':
                // TODO : login
                break;
            case 's':
                // TODO : remote-server
                break;
            case 'u':
                // TODO : username
                break;
            case 'p':
                // TODO : password
                break;
            case 'a':
                // TODO : all
                break;
            default:
                fprintf(stderr, "Option inconnue\n");
                exit(EXIT_FAILURE);
        }
    }

    printf("Programme exécuté sans option dry-run\n");
    return 0;
}

//---------------- Fonctions ----------------//
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
