#include <getopt.h>
#include <stdio.h>

/*
NB : struct option est déja une strucrue définie dans le getopt.h
struct option {
    const char *name;     // nom long : "help"
    int         has_arg;  // 0 : no_argument / 1 : required_argument / 2 : optional_argument
    int        *flag;     // soit NULL, soit un pointeur vers une variable
    int         val;      // valeur renvoyée par getopt_long (ex: 'h')
};
*/

/*
CMD exemple
gcc -o main .\getopt_long.c ; .\main.exe --org --binary 25 --app hi
*/


int main(int argc, char *argv[]) {
	int option = 0;
	struct option my_opts[] = {
		{.name="org",.has_arg=0,.flag=0,.val='o'},
		{.name="app",.has_arg=1,.flag=0,.val='a'},
		{.name="binary",.has_arg=2,.flag=0,.val='b'},
		{.name=0,.has_arg=0,.flag=0,.val=0}, // last element must be zero
	};
	while((option = getopt_long(argc, argv, "oa:b::", my_opts, 0)) != -1) {
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
	return 0;
}
