// L'objectif de cet exercie est de "recopier" l'exercie 2, mais au lieu de stocker les valeurs dans différentes variable, on les stockes toute
// dans parameter_t (plus propre)


#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

#define STR_MAX 1024
#define PARAMETER_BUFFER_SIZE 16



typedef enum {
	PARAM_VERBOSE, /* Verbose parameter */
	PARAM_DATA_SOURCE, /* Data source parameter */
	PARAM_TEMP_DIR, /* Temp directory parameter */
	PARAM_OUTPUT_FILE, /* Output file parameter */
	PARAM_CPU_MULT, /* CPU multiplier parameter */
} parameter_id_t;

typedef union {
	long long int_param; // For CPU mult
	bool flag_param; // For verbose
	char str_param[STR_MAX]; // For path: data, temp dir & output
} data_wrapper_t;

typedef struct {
	parameter_id_t parameter_type;
	data_wrapper_t parameter_value;
} parameter_t;




int main(int argc, char *argv[]) {
	parameter_t my_parameters[PARAMETER_BUFFER_SIZE];
	int parameters_count = 0; // Increment when receiving a param
	parameter_t default_parameters[] = {
		{.parameter_type=PARAM_VERBOSE, .parameter_value.flag_param=false},
		{.parameter_type=PARAM_DATA_SOURCE, .parameter_value.str_param="hi"},
		{.parameter_type=PARAM_TEMP_DIR, .parameter_value.str_param="./temp"},
		{.parameter_type=PARAM_OUTPUT_FILE, .parameter_value.str_param="test"},
		{.parameter_type=PARAM_CPU_MULT, .parameter_value.int_param=2},
	};
	int default_parameters_count = sizeof(default_parameters) / sizeof(parameter_t);
	/* Do stuff here */

    // Récupération des options (pseudo copie de l'exo 2 du TP6)
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
                my_parameters[parameters_count].parameter_type = PARAM_VERBOSE;
                my_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;
            
            case 'd':
                my_parameters[parameters_count].parameter_type = PARAM_DATA_SOURCE;
                strcpy(my_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 't':
                my_parameters[parameters_count].parameter_type = PARAM_TEMP_DIR;
                strcpy(my_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'o':
                my_parameters[parameters_count].parameter_type = PARAM_OUTPUT_FILE;
                strcpy(my_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'c':
                my_parameters[parameters_count].parameter_type = PARAM_CPU_MULT;
                my_parameters[parameters_count].parameter_value.int_param = 2;
                parameters_count++;
                break;
        }
    }

    // Modification des parametres (dans le default_parameters)
    for(int i=0 ; i<parameters_count ; i++) {
        parameter_t new_parameter = my_parameters[i];
        for(int j=0 ; j<default_parameters_count ; j++) {
            if(default_parameters[j].parameter_type == new_parameter.parameter_type) {
                default_parameters[j].parameter_value = new_parameter.parameter_value;
                j = default_parameters_count;
            }
        }
    }



    // Retour des options
    
    printf("Verbose mode is %s\n", default_parameters[0].parameter_value.flag_param ? "On" : "Off");
    printf("Data source is located on %s\n", default_parameters[1].parameter_value.str_param);
    printf("Temporary files will be located on %s\n", default_parameters[2].parameter_value.str_param);
    printf("Output file will be produces on  %s\n", default_parameters[3].parameter_value.str_param);
    printf("%d tasks will run in parallel\n", default_parameters[4].parameter_value.int_param);
    

	return 0;
}
