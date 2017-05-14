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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


#include "message.h"
#include "socket.h"

#ifndef _MEMOIRE_PARTAGEE_H_
#define _MEMOIRE_PARTAGEE_H_

typedef int semaphore;


typedef struct joueur{
	int fd;
	char nom[256];
	int send_ecart;
	Carte ecart[5];
	int score;
} Joueur;

typedef struct {
	Joueur clients[NOMBRE_JOUEURS_MAX];
	int nb_clients;
	Joueur* joueurs[NOMBRE_JOUEURS_MAX];
	int nb_joueurs;
	Carte papayoo;
	Carte pli_en_cours[NOMBRE_JOUEURS_MAX];
	int taille_pli_en_cours;
	Color couleur_tour;
}struct_partagee;

void init_mem_RC();
void cloturer_memoire();

void ecrire_memoire(struct_partagee data);

struct_partagee lire_memoire();

#endif