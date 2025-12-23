#include <stdio.h>
#include <string.h>

#include "header.h"

// Les données sur les processus des machines distante seront enregistré sous le même format que le local(), càd
// PID:PPID:STATE:USER:%PROC:%MEM:TPS_EXEC_MMS:CMD
// Dans le fichier correspondant à 




// Prototypes
void remote_config_network(Remote_host_data remote_server_data);
void solo_network(Remote_host_data remote_server_data);
int file_remote_host_data(Remote_host_data *remote_server_data, char* line);
void ssh_connexion(Remote_host_data remote_server_data);
void telnet_connexion(Remote_host_data remote_server_data);



void network(Remote_host_data remote_server_data) {
    if(remote_server_data.use_remote_config) remote_config_network(remote_server_data);
    else solo_network(remote_server_data);

    return;
}



// Connxions depuis le fichier de configration passé en paramètre
void remote_config_network(Remote_host_data remote_server_data) {
    char line[512];
    FILE *remote_config_file = fopen(remote_server_data.remote_config_path, "r");
    if(!remote_config_file) {
        write_error("Erreur avec l'ouverture du fichier remote_config_path dans remote_config_network", errno);
        return;
    }

    while(fgets(line, sizeof(line), remote_config_file) != NULL) {
        if(!file_remote_host_data(&remote_server_data, line)) {
            write_error("Erreur lors de la récupération des infos d'une des lignes du config_file dans remote_config_network()", errno);
            continue;
        }

        // Lancement de la connexion en ssh ou en telnet
        if(strcmp(remote_server_data.connexion_type, "ssh")) ssh_connexion(remote_server_data);
        if(strcmp(remote_server_data.connexion_type, "telnet")) telnet_connexion(remote_server_data);
    }
}





// Etablit la connexion avec SSH + écrit dans le [machine_user].txt les données sous le bon format (PID:PPID:...)
void ssh_connexion(Remote_host_data remote_server_data) {
    // Execution de la commande "ps ..." sur la machine à distancte + "enregistremment" du retour dans le tmp_file
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ssh %s@%s ps -eo pid=,ppid=,state=,user=,%%cpu=,%%mem=,etimes=,cmd=", remote_server_data.username, remote_server_data.server_ip);
    FILE *tmp_file = popen(cmd, "r");
    if(!tmp_file) {
        write_error("Erreur avec le popen() de ssh_connexion()", errno);
        return;
    }

    // Ouverture/Création du fichier [user]_[nom_serveur].txt pour enregistrer les données sous le format PID:PPID:STATE:USER:%PROC:%MEM:TPS_EXEC_S:CMD
    char save_file_name[256];
    sprintf(save_file_name, "%s_%s.txt", remote_server_data.username, remote_server_data.server_name);
    FILE *save_file = fopen(save_file_name, "w+");
    if(!save_file) {
        write_error("Erreur avec le fopen de save_file dans ssh_connexion()", errno);
        return;
    }

    // Récupération ligne par ligne du retour du "ps ..." sur la machine + enregistrement dans le user_nomMachine.txt
    while(fgets(cmd, sizeof(cmd), tmp_file) != NULL) {
        int pid, ppid;
        char state, username[128], proc_cmd[512];
        float cpu_ratio, mem_ratio;
        unsigned long lifetime_in_sec;
        sscanf(cmd, "%d %d %c %s %f %f %lu %[^\n]",&pid, &ppid, &state, &username, &cpu_ratio, &mem_ratio, &lifetime_in_sec, &proc_cmd); // ^ = négation --> %[^\n] = "lit tout sauf le \n"
        fprintf(save_file, "%d:%d:%c:%s:%.2f:%.2f:%lu:%s\n", pid, ppid, state, username, cpu_ratio, mem_ratio, lifetime_in_sec, proc_cmd);
    }
}





int file_remote_host_data(Remote_host_data *remote_server_data, char* line) {
    char tmp_line[512];
    strncpy(tmp_line, sizeof(tmp_line), line);
    tmp_line[strcspn(tmp_line, "\n")] = '\0';

    char *token = strtok(tmp_line, ":");
    int field = 1;

    while (token) {
        switch (field) {
            case 1: strncpy(remote_server_data->server_name, token, sizeof(remote_server_data->server_name)); break;
            case 2: strncpy(remote_server_data->server_ip, token, sizeof(remote_server_data->server_ip)); break;
            case 3: remote_server_data->port = atoi(token); break;
            case 4: strncpy(remote_server_data->username, token, sizeof(remote_server_data->username)); break;
            case 5: strncpy(remote_server_data->psw, token, sizeof(remote_server_data->psw)); break;
            case 6: strncpy(remote_server_data->connexion_type, token, sizeof(remote_server_data->connexion_type)); break;
            default: return 0;
        }
        token = strtok(NULL, ":");
        field++;
    }
    return (field == 7);

}








void telnet_connexion(Remote_host_data h){
    char cmd[1024];

    // Création de la commande pour telnet
    snprintf(cmd, sizeof(cmd), "(echo \"%s\"; sleep 1; echo \"%s\"; sleep 1; echo \"ps\"; sleep 1; echo \"exit\") | telnet %s", h.username, h.psw, h.server_ip);  
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        write_error("Erreur avec le poen de fp dans telent_connexion()", errno);
        return;
    }

    
    // Ouverture/Création du fichier [user]_[nom_serveur].txt pour enregistrer les données sous le format PID:PPID:STATE:USER:%PROC:%MEM:TPS_EXEC_S:CMD
    char save_file_name[256];
    sprintf(save_file_name, "%s_%s.txt", h.username, h.server_name);
    FILE *save_file = fopen(save_file_name, "w+");
    if(!save_file) {
        write_error("Erreur avec le fopen de save_file dans ssh_connexion()", errno);
        return;
    }

    // Récupération ligne par ligne du retour du "ps ..." sur la machine + enregistrement dans le user_nomMachine.txt
    while (fgets(cmd, sizeof(cmd), fp) != NULL) {
        int pid, ppid;
        char state, username[128], proc_cmd[512];
        float cpu_ratio, mem_ratio;
        unsigned long lifetime_in_sec;
        sscanf(cmd, "%d %d %c %s %f %f %lu %[^\n]",&pid, &ppid, &state, &username, &cpu_ratio, &mem_ratio, &lifetime_in_sec, &proc_cmd); // ^ = négation --> %[^\n] = "lit tout sauf le \n"
        fprintf(fp, "%d:%d:%c:%s:%.2f:%.2f:%lu:%s\n", pid, ppid, state, username, cpu_ratio, mem_ratio, lifetime_in_sec, proc_cmd);       
    }

    pclose(fp);
    return 0;
}
