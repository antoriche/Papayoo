/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */


#ifndef TYPES_H_
#define TYPES_H_

#define EMPTY_MSG 0

#define INSCRIPTION 1
#define ANNULER 2
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

#define TRUE   1
#define FALSE  0

#define SYS(call) ((call) == -1) ? (perror(#call ": ERROR"),exit(1)) : 0

typedef enum {COEUR, CARREAU, TREFLE, PIQUE, PAYOO} Color;

typedef struct carte{
	int valeur;
	Color couleur;
} Carte;

typedef struct message{
	int type;
	char message[256];
	Carte cartes[30];
} Message;

Message lire_message(int fd);
void envoyer_message(int client, Message message);
/*void envoyer_message(int client, int type, char* message);
void envoyer_message(int client, int type, Carte cartes[30]);*/

#endif