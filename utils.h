/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
#############################################################
*/
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TRUE   1
#define FALSE  0

#define CARTE_NULL 0

#define noirTexte 30
#define rougeTexte 31

#define noirFond 40
#define jauneFond 43
#define blancFond 47

#define couleurOn(A,B,C) printf("\033[%d;%d;%dm",A,B,C)
#define couleurOff() printf("\033[0m");

#define INSCRIPTION 1
#define ENVOI_PAQUET 3
#define JOUER_CARTE 4
#define ENVOI_POINTS 5

#define INSCRIPTION_OK 6
#define INSCRIPTION_KO 7
#define DEBUT_PARTIE 8
#define DISTRIBUTION_CARTES 9
#define DISTRIBUTION_PAQUET 10
#define DEMANDER_CARTE 11
#define AVERTIR_PLI_EN_COURS 12
#define ENVOI_PLI 13
#define COMPTER_POINTS 14
#define FIN_PARTIE 15
#define ANNULE 16

#define BAD_REQUEST 17
#define CONNECTION_FULL 18



#define SYS(call) ((call) == -1) ? (perror(#call ": ERROR"),exit(1)) : 0

typedef enum {COEUR=0, CARREAU=1, TREFLE=2, PIQUE=3, PAYOO=4} Color;

typedef struct carte{
	int valeur;
	Color couleur;
} Carte;

typedef struct message{
	int type;
	union {
		char message[256];
		Carte cartes[30];
	} data;
} Message;

Message lire_message(int fd);
void envoyer_message(int client, Message message);


void carte2str(Carte c);
Carte getRandomCarte(Carte* paquet,int *nb_cartes);
Carte* paquet(int* nb_cartes);
void afficher(int couleur);




#endif