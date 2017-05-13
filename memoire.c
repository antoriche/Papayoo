/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
#############################################################
*/
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define KEY 1000
#define MUTEX "/MUTEX"
#define BD "/BD"

#define ACTIVER_MEMOIRE_PARTAGEE 1 //Debug

#include "memoire.h"

sem_t *mutex;
sem_t *bd;
int rc = 0;
int mem_ID;
int shm;

void init_sem(){
	/*
	
	if((shm=shm_open("/shm",O_RDWR|O_CREAT,S_IRWXU))<0){
		perror("Erreur shm_open\n");
		exit(1);
	}

	if(ftruncate(shm,sizeof(sem_t))<0){
		perror("Erreur ftruncate\n");
		exit(1);
	}
	
	if(ftruncate(shm,sizeof(sem_t))<0){
		perror("Erreur ftruncate\n");
		exit(1);
	}

	if((bd=mmap(NULL,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED,shm,0))==MAP_FAILED){
		perror("Erreur mmap bd\n");
		exit(1);
	}if((mutex=mmap(NULL,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED,shm,0))==MAP_FAILED){
		perror("Erreur mmap mutex\n");
		exit(1);
	}
	if(sem_init(bd,1,1)==-1){
		perror("Erreur semaphore bd\n");
		exit(1);
	}
	if(sem_init(mutex,1,1)==-1){
		perror("Erreur semaphore mutex\n");
		exit(1);
	}*/


	if((bd = sem_open(BD,O_CREAT,0666,1))==NULL){
		perror("Erreur semaphore bd\n");
		exit(1);
	}
	if((mutex=sem_open(MUTEX,O_CREAT,0666,1))==NULL){
		perror("Erreur semaphore bd\n");
		exit(1);
	}
}


struct_partagee test;
void ecrire_memoire(struct_partagee data){
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
			return 0;
		}

		if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)	
		{
			perror("erreur shmat");											
			return 0;
		}
		*ptr_mem_partagee=data;

		shmdt(ptr_mem_partagee);

		sem_post(&bd);
}

struct_partagee lire_memoire(){
	if(!ACTIVER_MEMOIRE_PARTAGEE)return test;
	int mem_ID; 
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
		
		rc=rc+1;
		if(rc==1) sem_wait(&bd);
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
		rc=rc-1;
		if(rc==0) sem_post(&bd);
		sem_post(&mutex);

		data=(*ptr_mem_partagee);
		return data;
	
}


