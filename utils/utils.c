#include "utils.h"

int saisieString(char* str_chaine,int int_taille){
	char* ligne;
	size_t size_alloue;
	int int_tailleReelle;
	do{
		ligne = NULL;
		int_tailleReelle = getline(&ligne, &size_alloue,stdin);
		if (int_tailleReelle > int_taille){
			printf("La longueur ne doit pas excéder %d caractères.\n",int_taille-1);
		}
	}while(int_tailleReelle > int_taille || int_tailleReelle==1);
	strncpy(str_chaine,ligne,int_tailleReelle-1);
	str_chaine[int_tailleReelle-1] = '\0';
    return(int_tailleReelle);
}