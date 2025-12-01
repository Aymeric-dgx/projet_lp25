#include <stdio.h>
#include <dirent.h>
#include <ctype.h>


int main(int argc, char* argv []) {

    DIR* proc_directory = opendir("/proc");
    struct dirent* sub_directory;

    int i=0;
    while( (sub_directory = readdir(proc_directory)) != NULL) {
        // Si le nom du dossier ouvert commence par un nombre, alors cela veut dire que c'est un dossier lié à un processus (cf strucutre /proc)
        if( isdigit(sub_directory->d_name[0]) ) {
            
            // Récupération de la commande
            char location[512];
            char cmd[255]="";
            snprintf(location, sizeof(location), "/proc/%s/cmdline", sub_directory->d_name);
            FILE *cmd_file = fopen(location, "r");
            if(!cmd_file) {
                printf("Error cmd_file\n");
            } 
            else {
                fgets(cmd, sizeof(cmd), cmd_file);
                printf("%s\n", cmd);
            }
              
        }
    }


    return 0;
}
