/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
#############################################################
*/

#define KEY 1000
#include "memoire.h"

struct_partagee* init_memoire(struct_partagee data){
	int mem_ID;
	struct_partagee *ptr_mem_partagee;

	if ((mem_ID = shmget(KEY, sizeof(struct_partagee), IPC_CREAT | 0666)) < 0)	//	je crée un nouveau segment mémoire de taille "taille de ma structure data" octets, avec des droits d'écriture et de lecture
	{
		perror("erreur shmget");											//	et je m'assure que l'espace mémoire a été correctement créé
		return 0;
	}

	if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)	//	J'attache le segment de mémoire partagée identifié par mem_ID au segment de données du processus A dans une zone libre déterminée par le Système d'exploitation
	{
		perror("erreur shmat");											//	et je m'assure que le segment de mémoire a été correctement attaché à mon processus
		return 0;
	}

	return ptr_mem_partagee;
	



}

struct_partagee lire_memoire(){
	int mem_ID; //	identificateur du segment de mémoire partagée associé à CLEF (là encore le nom de cette variable n'a rien à voir avec celle du programme A mais son contenu sera évidemment identique)
	struct_partagee* ptr_mem_partagee; //	adresse d'attachement du segment de mémoire partagée (idem)

	//	J'instancie une structure "structure_partagee_B" et je l'appelle Data_B. Cela me sert uniquement à connaitre la taille de ma structure. Pour bien faire, il faudrait évidemment déclarer cette structure dans un .h qui serait inclu dans A et dans B avec la clef, de façon à garder la cohérence entre les 2 programmes
	struct_partagee data;

	if ((mem_ID = shmget(KEY, sizeof(struct_partagee), 0444)) < 0)	//	Je cherche le segment mémoire associé à CLEF et je récupère l'identificateur de ce segment mémoire... J'attribue des droits de lecture uniquement
	{
		perror("shmget");											//	et je m'assure que l'espace mémoire a été correctement créé
		exit(1);
	}
	if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)	//	J'attache le segment de mémoire partagée identifié par mem_ID_B au segment de données du processus B dans une zone libre déterminée par le Système d'exploitation
	{
		perror("shmat");											//	et je m'assure que le segment de mémoire a été correctement attaché à mon processus
		exit(1);
	}

	data=(*ptr_mem_partagee);
	
	return data;
}

void detacher(struct_partagee* ptr){
	if(shmdt(ptr)<0);
}
