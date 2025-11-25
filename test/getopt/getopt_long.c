#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

// ATTENTION : le faire sur linux, car sys/sysinfo.h n'existe pas sur windows

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
    int verbose = 0;
    char data_source[256] = "Undefined";
    char temporary_directory[256] = "Undefined";
    char output_file[256] = "Undefined";
    int cpu_multiplicator = 1;
    
	
	struct option options_list[] = {
        {"verbose", 0, 0, 'v'},
        {"data-source", 1, 0, 'd'},
        {"temporary-directory", 1, 0, 't'},
        {"output-file", 1, 0, 'o'},
        {"cpu-multiplier", 1, 0, 'c'}
    };
    int option = 0;
    while((option = getopt_long(argc, argv, "vd:t:o:c:", options_list, 0)) != -1) {
        switch(option) {
            case 'v':
                verbose = 1;
                break;
            
            case 'd':
                strcpy(data_source, optarg);
                break;

            case 't':
                strcpy(data_source, optarg);
                break;

            case 'o':
                strcpy(data_source, optarg);
                break;

            case 'c':
                cpu_multiplicator = atoi(optarg);
                break;
        }
    }
    char *str_verbose = (verbose == 1 ? "On" : "Off");

    printf("Verbose mode is %s\n", str_verbose);
    printf("Data source is located on %s\n", data_source);
    printf("Temporary files will be located on %s\n", temporary_directory);
    printf("Output file will be produces on  %s\n", output_file);
    printf("%d tasks will run in parallel\n", cpu_multiplicator*get_nprocs());

	return 0;
}
