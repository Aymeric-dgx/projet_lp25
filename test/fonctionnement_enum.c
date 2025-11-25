#include <stdio.h>

/*
Les enum permettent de créer des "mots clé" auxquelle on peut affecter des valeurs, de manière à ce que ce soit lisible pour un humain et
que les comparaisons, test etc soit plus efficace (permet de comparer des int au lieu de char --> BEAUCOUP plus simpe et rapide)
*/

enum naturel { 
    ZERO=5, 
    UN=6, 
    DEUX=7, 
    TROIS=8, 
    QUATRE=9, 
    CINQ=10 
};


int main(void) {
	printf("n = %d.\n", QUATRE);
	printf("UN = %d.\n", UN);
	return 0;
}
