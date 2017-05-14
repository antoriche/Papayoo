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
#define KEY_RC 2000
#define MUTEX "/MUTEX"
#define BD "/BD"

#define ACTIVER_MEMOIRE_PARTAGEE 1 //Debug

#include "memoire.h"

sem_t *mutex;
sem_t *bd;




void init_mem_RC(){
	int* rc;
	int mem_RC;
	if ((mem_RC = shmget(KEY_RC, sizeof(int), IPC_CREAT | 0666)) < 0)	
		{
			perror("erreur shmget");											
			exit(1);
		}
	if ((rc = (int*)shmat(mem_RC, NULL, 0)) == (int*) -1)	
		{
			perror("erreur shmat");											
			exit(1);
		}
	*rc=0;
	shmdt(rc);
}




struct_partagee test;
void ecrire_memoire(struct_partagee data){
	int mem_ID;
	if(!ACTIVER_MEMOIRE_PARTAGEE){
		test = data;
		return;
	}
	struct_partagee *ptr_mem_partagee;
	if((bd = sem_open(BD,O_CREAT,0666,1))==NULL){
		perror("Erreur semaphore bd\n");
		exit(1);
	}
	
	
		sem_wait(&bd);
		if ((mem_ID = shmget(KEY, sizeof(struct_partagee), IPC_CREAT | 0666)) < 0)	
		{
			perror("erreur shmget1");											
			exit(1);
		}

		if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)	
		{
			perror("erreur shmat");											
			exit(1);
		}
		*ptr_mem_partagee=data;

		shmdt(ptr_mem_partagee);

		sem_post(&bd);
}

struct_partagee lire_memoire(){
	if(!ACTIVER_MEMOIRE_PARTAGEE)return test;
	int mem_ID; 
	int mem_RC;
	int *rc;
	struct_partagee* ptr_mem_partagee; 
	struct_partagee data;
	if((bd = sem_open(BD,O_CREAT,0666,1))==NULL){
		perror("Erreur semaphore bd\n");
		exit(1);
	}
	if((mutex=sem_open(MUTEX,O_CREAT,0666,1))==NULL){
		perror("Erreur semaphore mutex\n");
		exit(1);
	}
	
		sem_wait(&mutex);

	if ((mem_RC = shmget(KEY_RC, sizeof(int), IPC_CREAT | 0666)) < 0)	
		{
			perror("erreur shmget");											
			exit(1);
		}
	if ((rc = (int*)shmat(mem_RC, NULL, 0)) == (int*) -1)	
		{
			perror("erreur shmat");											
			exit(1);
		}
	


		
		*rc=*rc+1;
		if(*rc==1) sem_wait(&bd);
		sem_post(&mutex);


		if ((mem_ID = shmget(KEY, sizeof(struct_partagee), 0444)) < 0)	
		{
			perror("shmget");											
			exit(1);
		}
		if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)
		{
			perror("shmat");											
			exit(1);
		}
		sem_wait(&mutex);
		*rc=*rc-1;
		if(*rc==0) sem_post(&bd);
		sem_post(&mutex);

		data=(*ptr_mem_partagee);
		return data;
	
}


