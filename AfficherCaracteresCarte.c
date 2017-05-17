// sequences "d'escape caracters" pour afficher les codes des cartes.
// mai 2016

#include <stdio.h>
#include <stdlib.h>

#define PIQUE  "\xE2\x99\xA0"
#define TREFLE "\xE2\x99\xA3"
#define COEUR  "\xE2\x99\xA5"
#define CARREAU  "\xE2\x99\xA6"

// un peu de couleur

#define noirTexte 30
#define rougeTexte 31
#define vertTexte 32
#define jauneTexte 33
#define bleuTexte 34
#define magentaTexte 35
#define cyanTexte 36
#define blancTexte 37

#define noirFond 40
#define rougeFond 41
#define vertFond 42
#define jauneFond 43
#define bleuFond 44
#define magentaFond 45
#define cyanFond 46
#define blancFond 47


#define couleurOn(A,B,C) printf("\033[%d;%d;%dm",A,B,C)
#define couleurOff() printf("\033[0m");


void afficher(int couleur){
    switch(couleur){
        case 0 : 
            couleurOn(0,rougeTexte,noirFond);
            printf (COEUR);
            printf("\n");
            couleurOff();
            break;
        case 1 : 
            couleurOn(0,rougeTexte,noirFond);
            printf (CARREAU);
            printf("\n");
            couleurOff();
            break;
        case 2 : 
            couleurOn(0,noirTexte,blancFond);
            printf (TREFLE);
            printf("\n");
            couleurOff();
            break;
        case 3 : 
            couleurOn(0,noirTexte,blancFond);
            printf (PIQUE);
            printf("\n");
            couleurOff();
            break;
        case 4 : 
            couleurOn(0,bleuTexte,jauneFond);
            printf ("PAYOO");
            printf("\n");
            couleurOff();
            break;
    }
    return;
}
