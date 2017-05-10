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

#include "serveur.h"
#include "message.h"

typedef struct {
	Joueur *joueurs;
	Carte papayoo;
	Carte *pli;
}struct_partagee;

int envoyer_donnees(struct_partagee data);

struct_partagee recevoir_donnees();