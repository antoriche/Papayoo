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
#include "socket.h"

typedef struct joueur{
	int fd;
	char nom[256];
	int send_ecart;
	Carte ecart[5];
	int score;
} Joueur;

typedef struct {
	Joueur* joueurs[NOMBRE_JOUEURS_MAX];
	int nb_joueurs;
	Carte papayoo;
	Carte* pli_en_cours[NOMBRE_JOUEURS_MAX];
	int taille_pli_en_cours;
	Color couleur_tour;
}struct_partagee;

struct_partagee* init_memoire();

struct_partagee lire_memoire();

void detacher(struct_partagee* ptr);