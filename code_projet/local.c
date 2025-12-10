// NB TRES IMPORTZNT
// Vérifier avant l'enregistrement dans le .txt que la commande est != de "x86_Thread_features_locked:" (avec un \n peut être ?)
// Si c'est le cas, ne pas l'enregistrer dans la liste des processus à retourner

#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>


typedef struct {
    int pid;
    int ppid;
    char user[256];
    char state;
    float cpu_rate;
    float mem_rate;
    unsigned long lifetime;
    char cmd[256];

    unsigned long fisrt_proc_ticks; // Pour calculer le %CPU
} Proc;


// Prototypes des fonctions que l'on va utiliser
unsigned long get_proc_lifetime(int pid);
void get_all_cpu_rate_procs(Proc* proc_list);
void write_procs_in_txt(FILE* file_where_write, Proc* proc_list, int nb_procs);
void local(FILE* returning_file);



int main(int argc, char* argv[]) {
    FILE* file = fopen("temporary_text.txt", "w+");
    if(!file) {
        printf("error with the file in the main()");
    }

    local(file);

    return 1;
}








void local(FILE* returning_file) {
    char line[512]; // Buffer globale permettant de réupérer les lignes de .txt

    // Ouverture du dossier /proc
    DIR* proc_dir = opendir("/proc");
    if(!proc_dir) {
        printf("Error with the opend() of /proc in local()");
        return;
    }
    struct dirent* sub_directory;

    // Récupération de la taille de la mémoire globale
    float global_mem_size;
    FILE* proc_meminfo_file = fopen("/proc/meminfo", "r");
    fgets(line, sizeof(line), proc_meminfo_file);
    sscanf(line, "MemTotal: %f", &global_mem_size);
    fclose(proc_meminfo_file);




    // 1ere boucle pour récupérer les valeurs "statiques" : PID, PPID, STATE, %MEM, CMD, LIFETIME + création dynamique du tableai Proc* list_proc
    Proc* proc_list = malloc(sizeof(Proc));
    int uid; // Pour trouver le user, en faisant user = getpwuid(uid)
    int index = 0;

    while ( (sub_directory = readdir(proc_dir)) != NULL) {
        if(isdigit(sub_directory->d_name[0])) {

            //----------------------- Récupération/Calcul de PID, PPID, STATE, %MEM, LIFETIME -----------------------------//
            char loc[521];
            snprintf(loc, sizeof(loc), "/proc/%s/status", sub_directory->d_name);
            FILE* status_file = fopen(loc, "r");
            if(!status_file) printf("Error with fopen() of status_file");

            float proc_mem_size;
            int pid = atoi(sub_directory->d_name);


            // Parcours de /proc/[PID]/status
            while(fgets(line, sizeof(line), status_file) != NULL) { 
                sscanf(line, "State: %c", &proc_list[index].state);
                sscanf(line, "Pid: %d", &proc_list[index].pid);
                sscanf(line, "PPid: %d", &proc_list[index].ppid); 
                sscanf(line, "VmRSS: %f", &proc_mem_size);
                sscanf(line, "Uid: %d", &uid);
            }


            // Calcul/Enregistrement de %MEM + LIFETIME 
            float mem_ratio = proc_mem_size / global_mem_size * 100; // La RAM, pas la memoire de stockage
            proc_list[index].mem_rate = mem_ratio;
            proc_list[index].lifetime = get_proc_lifetime(pid);


            // Récupération de CMDLINE
            snprintf(loc, sizeof(loc), "/proc/%s/cmdline", sub_directory->d_name);
            FILE* cmd_file = fopen(loc, "r");
            if(!cmd_file) printf("Error with fopen() of cmd_file in local()");
            fgets(line, sizeof(line), cmd_file);
            strcpy(proc_list[index].cmd, line);


            // Récupératon de USER
            struct passwd* pw = getpwuid(1000);
            strcpy(proc_list[index].user, pw->pw_name);

            // Fermeture des fichiers
            fclose(status_file);
            fclose(cmd_file);

            // Incrémentation de index + realocation de mémoire à proc_list
            index++;
            proc_list = realloc(proc_list, sizeof(Proc) * (index+1));
        }
    }

    // Calcul du %CPU (doit être fait à part)
    get_all_cpu_rate_procs(proc_list);


    // Ecriture des processus dans un .txt selon le format vu avec Nathan
    // NB : les "faux" processus avec la commande "x86_Thread_features_locked:" sont ignorées dans la fonction
    write_procs_in_txt(returning_file, proc_list, index-1);
}


//--------------------------------------------------------- Fonctions --------------------------------------------------------//



// Permet d'obtenir le nb de SECONDES passées depuis le démarrage du processeur
double get_uptim_system() {
    double uptime_system_sec; // Directement en sec
    FILE* proc_uptime_file = fopen("/proc/uptime", "r");
    if(!proc_uptime_file) {
        printf("Error by trying to open /proc/uptime in get_uptime_system()");
        return 0;
    }

    fscanf(proc_uptime_file, "%lf", &uptime_system_sec);
    return uptime_system_sec;
}




// Permet d'obtenir la date de démarrage d'un proccessus en nb de TICKS passé depuis le démarage du processeur (ex : démmaré 10 sec après le démaragge du pc --> starttime = 1000 ticks)
unsigned long get_proc_startime(int pid) {
    char loc[512];
    unsigned long proc_startime_ticks; // En nb de ticks

    sprintf(loc, "/proc/%d/stat", pid);
    FILE* proc_pid_stat_file = fopen(loc, "r");
    if(!proc_pid_stat_file) {
        printf("Error by trying to open /proc/pid/stat in get_proc_startime(int pid)");
        return 0;
    }

    fgets(loc, sizeof(loc), proc_pid_stat_file);
    sscanf(loc, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu", &proc_startime_ticks);

    return proc_startime_ticks;
}


// retourne la durée de vie en temps réel en secondes d'un processus
unsigned long get_proc_lifetime(int pid) {
    // lifetime = uptime - starttime
    unsigned long uptime_nb_ticks = get_uptim_system() * sysconf(_SC_CLK_TCK);
    unsigned long lifetime = (uptime_nb_ticks - get_proc_startime(pid)) / sysconf(_SC_CLK_TCK);

    return lifetime;
}








// Retourne le nb de ticks "produit" par le cpu depuis le début
unsigned long get_cpus_ticks() {
    FILE* proc_stat_file = fopen("/proc/stat", "r");
    if(!proc_stat_file) {
        printf("Erreur avec le fopen du /proc/stat dans get_cpus_ticks()\n");
        return 1;
    }
    char line[512];
    fgets(line, sizeof(line), proc_stat_file);
    fclose(proc_stat_file);

    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    return user + nice + system + idle + iowait + irq + softirq + steal;
}





// Retourne le nb de ticks "consommé" par un processus depuis son démarrage
unsigned long get_proc_ticks(int pid) {
    char line[512];
    snprintf(line, sizeof(line), "/proc/%d/stat", pid);
    FILE* proc_pid_stat_file = fopen(line, "r");
    if(!proc_pid_stat_file) {
        printf("Erreur avec le fopen du /proc/pid/stat dans get_proc_ticks()\n");
        return 1;
    }

    fgets(line, sizeof(line), proc_pid_stat_file);
    fclose(proc_pid_stat_file);

    unsigned long u_ticks, k_ticks, child_user_time, child_system_time;
    sscanf(line, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu %lu %lu", &u_ticks, &k_ticks, &child_user_time, &child_system_time);
    
    return u_ticks + k_ticks + child_user_time + child_system_time;
}



// Récupération / calcul des %CPU de tout les processus
void get_all_cpu_rate_procs(Proc* proc_list) {
    int nb_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    DIR* proc_dir = opendir("/proc");
    if(!proc_dir) {
        printf("Erreur avec le opend d /proc dans get_all_cpu_rate_procs()\n");
        return;
    }
    struct dirent* sub_directory;


    // 1ere boucle
    int index;
    unsigned long first_cpu_ticks = get_cpus_ticks();

    while( (sub_directory = readdir(proc_dir)) != NULL) {
        if(isdigit(sub_directory->d_name[0])) {
            int pid = atoi(sub_directory->d_name);
            index = 0;

            while(proc_list[index].pid != pid) index++;
            proc_list[index].fisrt_proc_ticks = get_proc_ticks(pid);
        }
    }

    // Pause avec nanosleep()
    struct timespec tim;
    tim.tv_sec = 2;
    tim.tv_nsec = 0;
    nanosleep(&tim, NULL);


    // 2eme boucle
    rewinddir(proc_dir); // Reset du curseur dans le proc_dir pour pouvoir le reparcourir
    unsigned long cpu_delta = get_cpus_ticks() - first_cpu_ticks;

    while( (sub_directory = readdir(proc_dir)) != NULL) {
        if(isdigit(sub_directory->d_name[0])) {
            int pid = atoi(sub_directory->d_name);
            index = 0;
            
            while(proc_list[index].pid != pid) index++;
            float cpu_rate = (float) (get_proc_ticks(pid) - proc_list[index].fisrt_proc_ticks) / cpu_delta;  // proc_delta / cpu_delta
            proc_list[index].cpu_rate = cpu_rate * 100 * nb_cpus;
        }
    }
}




void write_procs_in_txt(FILE* file_where_write, Proc* proc_list, int nb_procs) {
  
    
    // Parcours de procs_list
    for(int i=0 ; i<nb_procs ; i++) {
        // SSI la cmdline est != de "x86_Thread_features_locked:", l'ajouter au .txt
        if(strncmp(proc_list[i].cmd, "x86_Thread_features_locked:", 24) != 0) {
            fprintf(file_where_write, "%d:%d:%c:%s:%.2f:%.2f:%lu:%s\n",
                proc_list[i].pid,
                proc_list[i].ppid,
                proc_list[i].state,
                proc_list[i].user,
                proc_list[i].cpu_rate,
                proc_list[i].mem_rate,
                proc_list[i].lifetime,
                proc_list[i].cmd);
        }
    }


}
