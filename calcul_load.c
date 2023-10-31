#include<stdlib.h>
#include<stdio.h>


//collisions entre deux trucs de taille i et j coute i+j+1 position
//on peut améliorer de 1 avec des contraintes d'ordre des restes:
//on place des k uples dans l'ordre des restes. Le conflit avec les autres k uples
// dans la deuxième période interdit 2k emplacements 
//on peut toujours garantir ça en plaçant d'abord toujours les premiers k-uples
//possible (en revenant en arrière dans les paquetes non sélectionnés) et quand ça bloque on fait dans l'ordre
//quand on place des singletons, on peut supposer qu'ils sont de plus grand reste
//par rapport à tout ce qui a déjà été placé
// ça permet de dire que les k-uples leurs interdisent au plus  k+1 places

//si on est capable de toujours garantir que les messages critiques sont de reste plus
//grand que ceux placés avant on peut enlever 1 partout
//->ça peut se calculer en pratique

int main(int argc, char **argv){
	int k = atoi(argv[1]);
	double* message_number = malloc(sizeof(double)*(k+1));
	double* message_number_critique = malloc(sizeof(double)*(k+1));
	for(int i = k; i>1; i--){
		double val = 0;
		for(int j = k; j > i ; j--){
			val += message_number[j] * (i*j + i + j +1) / j;
		}
		message_number[i] =  (1 - val) / (i + 2); //+ (double)1/i pour plus que 3 seulement?
	}
	//calcul pour 2 à part ?

	//calcul de message_number[1] de manière plus précise
	double val = 0;
		for(int j = k; j > 1 ; j-- ){
			val += message_number[j] * (2*j + 1) / j;
		}
		message_number[1] =  (1 - val) / 3;
	double res = 0;
	for(int i = 1; i<= k; i++){
		res += message_number[i];
	}
	printf("Résultat pour des %d-uples: %f \n ",k,res);
	for (int i = 1; i<=k; i++) printf("%f ",message_number[i]);
	return 0;
}
