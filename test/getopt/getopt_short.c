#include <getopt.h>
#include <stdio.h>

/* 
CMD exemple
gcc -o main .\getopt_short.c ; .\main.exe -o -b -a hello
*/


/* NB : valeur de optind = 1, puis :
    +1 si c'est une option sans argument
    +1 si c'est une option avec argument OPTIONNEL
    +2 si c'est une option avec argument OBLIGATOIRE

En sachant que les +1/2 se font dans le getopt

Il y'a bien un optarg pour les options avec arg obligatoire, mais pas pour les optionnels (sauf si c'est collé, ex : -b25)
*/

int main(int argc, char **argv) {

    for(int i=0 ; i<argc ; i++) {
        printf("%s\n", argv[i]);
    }
    printf("\n");


    int option = 0;
    while( (option = getopt(argc, argv, "oa:b::")) != -1) {
        //printf("Puis option = %c optind = %d, optarg = %s\n", option, optind, optarg);
        switch(option) {
            case 'o':
                printf("-o active\n"); 
                break;
            case 'a':
                printf("-a active with as argument %s\n", optarg);
                break;
            case 'b':
                printf("-b active ");
                // Cas possible : -b, -b25, -b 25 (avec potentiellement d'autre options après)

                // Si l'argument est collé (ex : -b25), on fait comme pour -a
                if(optarg != NULL) {
                    printf("with %s\n", optarg);
                }

                // Sinon, on sait que le optind pointe l'argument (ou option) juste après le -b (car argument optionnel donc +1 et pas +2)
                else {
                    // Si on n'a pas atteint la fin ET que l'argument suivant n'est pas une option, alors cela veut dire que c'est l'argumenent de b
                    if(optind < argc && argv[optind][0] != '-') { 
                        printf("with %s\n", argv[optind]);
                        optind++; // On fait optind++ afin de pointer sur l'option suivant (ex : -b 25 -a, ainsi on atteindra le -a)
                    }
                    // Sinon, b est activé sans argument
                    else printf("without argument\n");
                }
                break;
        }
    }
    
    // Affichage des arguements restants
    if(optind < argc) {
        for(int i=optind ; i<argc ; i++)
        printf("Option restantes numero %d : %s\n", i, argv[i]);
    }


    return 0;
}
