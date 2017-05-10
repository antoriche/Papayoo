/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "message.h"

typedef struct joueur{
	int fd;
	char nom[256];
	int send_ecart;
	Carte ecart[5];
} Joueur;

typedef struct {
	Joueur *joueurs;
	Carte papayoo;
	Carte *pli;
}struct_partagee;

struct_partagee* envoyer_donnees();

struct_partagee recevoir_donnees();

//void detacher(struct_partagee* ptr)