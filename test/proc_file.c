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
            printf("Processus avec PID = %d\n", sub_directory->d_name);
            i++;
        }
    }
    printf("\nIl y'a %d processus", i);

    return 0;
}
