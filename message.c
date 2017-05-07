#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

Message lire_message(int fd){
	Message message;
	int l = read(fd,&message, sizeof(Message));
	if(l <= 0){
		Message e = {ANNULE,"\0"};
		return e;
	}
	return message;
}

void envoyer_message(int client, Message message){
	SYS(write(client,&message,sizeof(Message)));
}

char* carte2str(Carte c){
	char* ret;
	if( (ret = (char*)malloc(sizeof(char)*15)) == NULL){
		perror("Erreur malloc");
		exit(2);
	}
	sprintf(ret, "%d ", c.valeur);
	switch(c.couleur){
		case CARREAU:
			strcat(ret,"de carreau");
			break;
		case PIQUE:
			strcat(ret,"de pique");
			break;
		case TREFLE:
			strcat(ret,"de trefle");
			break;
		case COEUR:
			strcat(ret,"de coeur");
			break;
		case PAYOO:
			strcat(ret,"de payoo");
			break;
	}
	return ret;
}

Carte getRandomCarte(Carte* paquet, int* nb_cartes){
	Carte ret = {};

	if(*nb_cartes == 0){
		ret.valeur = -1;
		return ret;
	}

	int x = rand()%(*nb_cartes);
	memcpy(&ret,&paquet[x],sizeof(Carte));
	paquet[x] = paquet[*nb_cartes];

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
		cartes[i].valeur = i;
		cartes[i].couleur = PAYOO;
	}
	return cartes;
}