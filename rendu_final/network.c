#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include <time.h>


#include "header.h"


// Les données sur les processus des machines distante seront enregistré sous le même format que le local(), càd
// PID:PPID:STATE:USER:%PROC:%MEM:TPS_EXEC_MMS:CMD
// Dans le fichier correspondant à user_machine.txt


// Prototypes
void network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH], int* p_nb_network_user, int* p_stop_prog); // Fonction principale
void solo_network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH],  int* p_nb_network_user, int* p_stop_prog); // La fonction netwrok si le user utlise les -l, -u, -p ... Et pas le -c
void multi_network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH],  int* p_nb_network_user, int* p_stop_prog); // La fonction executé par network si le user utilise -c
// void first_telnet_connexion(Remote_host_data remote_server_data, char* socket_file_name, int size); // Idem que pour le first_ssh_connexion


// Fonction "pivot" qui sert à rediriger entre la connexion unique et les connexions multiples
void network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH], int* p_nb_network_user, int* p_stop_prog) {
    if(remote_server_data.use_remote_config) multi_network(remote_server_data, users_name, p_nb_network_user, p_stop_prog);
    if(!remote_server_data.use_remote_config) solo_network(remote_server_data, users_name, p_nb_network_user, p_stop_prog);
}


// Connexion a une seule machine à distance
// NB : si la connexion a été fait via le -l, -u, ... Alors le Remote_hostdata est déjà complété avec les données de connexions
void solo_network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH], int* p_nb_network_user, int* p_stop_prog) {
    // ===================== Connexion session ssh =================== //
    ssh_session session;
    ssh_channel channel;
    int rc;

    // Création de la session
    session = ssh_new();
    if (session == NULL) {  
        write_error("Erreur lors de la création de la session ssh dans solo_network de network.c", errno);
        return;
    }

    // Configuration de la session
    ssh_options_set(session, SSH_OPTIONS_HOST, remote_server_data.server_ip);
    ssh_options_set(session, SSH_OPTIONS_USER, remote_server_data.username);
    ssh_options_set(session, SSH_OPTIONS_PORT, &remote_server_data.port);

    // Connexion de la session ssh
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        write_error("SSH connect error: (CF ligne ci dessous)", errno);
        write_error(ssh_get_error(session), errno);
        ssh_free(session);
        return;
    }

    // Authentification
    rc = ssh_userauth_password(session, NULL, remote_server_data.psw);
    if (rc != SSH_AUTH_SUCCESS) {
        write_error("SSH auth error: (CF ligne ci dessous)", errno);
        write_error(ssh_get_error(session), errno);
        ssh_free(session);
        return;
    }


    // Passage du nb_network_user à 1, et ajout du fichier user_machine.txt à la liste des fichiers à lire par le ui() 
    *p_nb_network_user = 1;
    strcpy(users_name[*p_nb_network_user-1], filename);

    // ============= Boucle principale pour récupérer et envoyer les données des processus ==================== //
    while(!*p_stop_prog) {
        // Création d'un channel
        channel = ssh_channel_new(session);
        if (!channel) {
            write_error("ssh_channel_new failed", errno);
            break;
        }

        ssh_channel_open_session(channel);
        if (rc != SSH_OK) {
            write_error(ssh_get_error(session), errno);
            break;
        }

        // Commande à distance
        ssh_channel_request_exec(channel, "ps -eo pid=,ppid=,state=,user=,%cpu=,%mem=,etimes=,cmd=");
        if (rc != SSH_OK) {
            write_error(ssh_get_error(session), errno);
            break;
        }


        // Lecture du résultat
        char buffer[1024];
        int n;
        char filename[MEDIUM_STR_LENGTH];
        snprintf(filename, sizeof(filename), "%s_%s.txt", remote_server_data.username, remote_server_data.server_ip);
        FILE* output_file = fopen(filename, "w");
        if(!output_file) {
            write_error("Erreur lors de l'ouverture de output_file dans solo_network(), dans network.c", errno);
            break;
        }

        

    
        // =============== Boucle principale pour la lecture et enregistrement des données des processus ======================== //
        char recv_buf[1024];
        char line_buf[2048];
        int line_len = 0;

        while (!ssh_channel_is_eof(channel)) {
            n = ssh_channel_read(channel, recv_buf, sizeof(recv_buf), 0);
            if (n < 0) {
                write_error("ssh_channel_read failed", errno);
                break;
            }

            for (int i = 0; i < n; i++) {
                if (recv_buf[i] == '\n') {
                    line_buf[line_len] = '\0';

                    int pid, ppid;
                    char state[8], user[32], cmd[256];
                    float cpu, mem;
                    long etime;

                    if (sscanf(line_buf, "%d %d %s %s %f %f %ld %[^\n]",
                               &pid, &ppid, state, user,
                               &cpu, &mem, &etime, cmd) == 8) {

                        pthread_mutex_lock(&main_mutex);
                        fprintf(output_file,
                                "%d:%d:%s:%s:%.1f:%.1f:%ld:%s\n",
                                pid, ppid, state, user,
                                cpu, mem, etime, cmd);
                        pthread_mutex_unlock(&main_mutex);
                    }

                    line_len = 0;
                } 
                else if (line_len < (int)sizeof(line_buf) - 1) {
                    line_buf[line_len++] = recv_buf[i];
                }
            }
        }

        fclose(output_file);


        // Fermeture du chanel
        ssh_channel_send_eof(channel);
        ssh_channel_close(channel);
        ssh_channel_free(channel);

        sleep(3);   
    }

    // Fermeture propre de la session
    ssh_disconnect(session);
    ssh_free(session);
}


// Connexion à une ou plusieurs machines à distance
void multi_network(Remote_host_data remote_server_data, char users_name[][MEDIUM_STR_LENGTH], int* p_nb_network_user, int* stop_prog) {
    // Parcours du fichier de configuration, et enregistrement des données de la machine (username, ip_machine, psw, ...) --> NB : Une ligne = une machine, et une ligne ressemble à ceci --> nom_serveur1:adresse_serveur:port:username:password:type_connexion1

    // Création des sessions (une pour chaque machine donc ?)

    // Récupération en boucle des infos sur les processus et enregsitrement dans le user_machine.txt (un par machine)

    // fermeture des sessions
}
