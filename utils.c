/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */

#include "utils.h"





Message lire_message(int fd){
	Message message;
	int l = read(fd,&message, sizeof(Message));
	if(l <= 0){
		Message e = {ANNULE,{"\0"}};
		return e;
	}
	return message;
}

void envoyer_message(int client, Message message){
	SYS(write(client,&message,sizeof(Message)));
}



void carte2str(Carte c){
	char* ret;
	if( (ret = (char*)malloc(sizeof(char)*15)) == NULL){
		perror("Erreur malloc\n");
		exit(2);
	}
	printf("%d ", c.valeur);
	switch(c.couleur){
        case COEUR : 
            couleurOn(0,rougeTexte,noirFond);
            printf("\xE2\x99\xA5 ");
            
            couleurOff();
            break;
        case CARREAU : 
            couleurOn(0,rougeTexte,noirFond);
            printf("\xE2\x99\xA6 ");
            couleurOff();
            break;
        case TREFLE : 
            couleurOn(0,noirTexte,blancFond);
            printf("\xE2\x99\xA3 ");
            couleurOff();
            break;
        case PIQUE : 
            couleurOn(0,noirTexte,blancFond);
            printf("\xE2\x99\xA0 ");
            couleurOff();
            break;
        case PAYOO : 
            couleurOn(0,noirTexte,jauneFond);
            printf("\xE2\x98\xA2 ");
            couleurOff();
            break;
    }
	
}

Carte getRandomCarte(Carte* paquet, int* nb_cartes){
	Carte ret = {};

	if(*nb_cartes == 0){
		ret.valeur = CARTE_NULL;
		return ret;
	}

	int x = rand()%(*nb_cartes);
	int i;
	ret = paquet[x];
	paquet[x] = paquet[*nb_cartes-1];

	*nb_cartes-=1;
	return ret;
}

Carte* paquet(int* nb_cartes){
	*nb_cartes = 60;
	Carte* cartes;
	if( (cartes = (Carte*)malloc(sizeof(Carte)*(*nb_cartes))) == NULL){
		perror("Erreur malloc");
		exit(2);
	}
	int i = 0;

	int couleur = 0,valeur = 1;
	for( i = 0 ; i < 40 ; i++){
		cartes[i].valeur = valeur;
		cartes[i].couleur = couleur;
		valeur++;
		if(valeur > 10){
			valeur = 1;
			couleur++;
		}
	}
	for( i = 40 ; i < 60 ; i++){
		cartes[i].valeur = i-39;
		cartes[i].couleur = PAYOO;
		
	}
	return cartes;
}

void afficher(int couleur){
    switch(couleur){
        case COEUR : 
            couleurOn(0,rougeTexte,noirFond);
            printf("\xE2\x99\xA5 ");
            couleurOff();
            printf("\n");
            break;
        case CARREAU : 
            couleurOn(0,rougeTexte,noirFond);
            printf("\xE2\x99\xA6 ");
            couleurOff();
            printf("\n");
            break;
        case TREFLE : 
            couleurOn(0,noirTexte,blancFond);
            printf("\xE2\x99\xA3 ");
            couleurOff();
            printf("\n");
            break;
        case PIQUE : 
            couleurOn(0,noirTexte,blancFond);
            printf("\xE2\x99\xA0 ");
            couleurOff();
            printf("\n");
            break;
        case PAYOO : 
            couleurOn(0,noirTexte,jauneFond);
            printf("\xE2\x98\xA2 ");
            couleurOff();
            printf("\n");
            break;
    }
    return;
}

void clear(){
  system("clear");
}