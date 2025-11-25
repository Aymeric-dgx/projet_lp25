#include <stdio.h>

enum naturel { 
    ZERO=5, 
    UN=6, 
    DEUX=7, 
    TROIS=8, 
    QUATRE=9, 
    CINQ=10 
};


int main(void) {

	enum naturel n = ZERO;

	printf("n = %d.\n", n);
	printf("n = %d.\n", QUATRE);
	printf("UN = %d.\n", UN);
	return 0;
}
