#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>



int can_signal(int pid); // Si retourne : 0 --> OK, -1 --> Pas l'accès, -2 --> Processus inexistant


int main(int argc, char* argv[]) {

    int pid = 4111;
    int pid2 = 4147;



    if(can_signal(pid) == 0){
        // Pause le processus
        kill(pid, SIGSTOP);
        printf("Processus mis en pause\n");
        sleep(2);


        // Reprend le processus
        kill(pid, SIGCONT);
        printf("Processus remis en route\n");
        sleep(2);


        // Tue le processus
        kill(pid, SIGKILL);
        printf("Processus tué\n");
    }
    else printf("Processus 1 inexistant / pas les droits\n");

    sleep(5);

    if(can_signal(pid2) == 0) {
        // Arrete le processus (un peu comme CTRL+C)s
        kill(pid, SIGTERM);
        printf("Processus stoppé (comme CTRL+C)\n");
    }
    else printf("Processus 2 inexistant / pas les droits\n");
}



// Vérifie si on peut envoyer un signal au processus
int can_signal(int pid) {
    if (kill(pid, 0) == 0) return 1;        // OK
    if (errno == EPERM) return 0;          // Pas les droits
    if (errno == ESRCH) return -1;         // Processus n'existe pas
    return -2;                             // Autre erreur
}
