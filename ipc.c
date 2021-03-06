/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
#############################################################
*/

#define KEY 10062
#define KEY_RC 10068
#define MUTEX "/mutex"
#define BD "/bd"

#define ACTIVER_MEMOIRE_PARTAGEE 1 //Debug

#include "ipc.h"



void init_mem(){
	sem_t *mutex;
	sem_t *bd;
	int *rc;
	int mem_RC;
	

	SYS(sem_open(BD,O_CREAT,0666,1));
	SYS(sem_open(MUTEX,O_CREAT,0666,1));
	

	

	SYS(mem_RC = shmget(KEY_RC, sizeof(int), IPC_CREAT | 0666));	
		
	if ((rc = (int*)shmat(mem_RC, NULL, 0)) == (int*) -1)	
		{
			perror("erreur shmat");											
			exit(1);
		}
	*rc=0;
	shmdt(rc);

}





void cloturer_memoire(){
	int mem_RC;
	int mem_ID;
	sem_t *mutex;
	sem_t *bd;

	SYS(mem_RC = shmget(KEY_RC, sizeof(int), IPC_CREAT | 0666));
	SYS(shmctl(mem_RC,IPC_RMID,NULL));
	SYS(mem_ID = shmget(KEY, sizeof(struct_partagee), IPC_CREAT | 0666));
	SYS(shmctl(mem_ID,IPC_RMID,NULL));

	if((bd = sem_open(BD,O_CREAT,0666,1))==NULL){
		printf("Erreur semaphore bd 1\n");
		exit(1);
	}
	if((mutex=sem_open(MUTEX,O_CREAT,0666,1))==NULL){
		printf("Erreur semaphore mutex\n");
		exit(1);
	}
	sem_unlink(bd);
	sem_unlink(mutex);

}




struct_partagee test;
void ecrire_memoire(struct_partagee data){
	if(!ACTIVER_MEMOIRE_PARTAGEE){
		test = data;
		return;
	}
	sem_t *bd;

	int mem_ID;
	
	struct_partagee *ptr_mem_partagee;
	bd=sem_open(BD,0);
	SYS(mem_ID = shmget(KEY, sizeof(struct_partagee), IPC_CREAT | 0666));	
	
	
	sem_wait(bd);
	
	if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1)	
	{
		perror("erreur shmat");											
		exit(1);
	}
	*ptr_mem_partagee=data;
	shmdt(ptr_mem_partagee);
	sem_post(bd);
}

struct_partagee lire_memoire(){
	if(!ACTIVER_MEMOIRE_PARTAGEE)return test;

	sem_t *mutex;
	sem_t *bd;
	int *rc;
	
	int mem_ID; 
	int mem_RC;
	
	struct_partagee* ptr_mem_partagee; 
	struct_partagee data;

	SYS(bd = sem_open(BD,0));
	SYS(mutex = sem_open(MUTEX,0));

	

	if((mem_ID = shmget(KEY, sizeof(struct_partagee), 0444)) < 0){
		struct_partagee ret;
		ret.memoire_valide = 0;
		return ret;
	}
	SYS(mem_RC = shmget(KEY_RC, sizeof(int), IPC_CREAT | 0666));
	
	
	sem_wait(mutex);
		
	if ((rc = (int*)shmat(mem_RC, NULL, 0)) == (int*) -1){
		perror("erreur shmat");											
		exit(1);
	}

		
	*rc++;
	if(*rc==1){
		sem_wait(bd);
	} 
	sem_post(mutex);
	
	if ((ptr_mem_partagee = (struct_partagee*)shmat(mem_ID, NULL, 0)) == (struct_partagee*) -1){
		perror("shmat");					
		exit(1);
	}
	data=(*ptr_mem_partagee);

	sem_wait(mutex);
	*rc--;
	if(*rc==0){ 
		sem_post(bd);
	}
	sem_post(mutex);

	
	shmdt(rc);
	shmdt(ptr_mem_partagee);
	data.memoire_valide = 1;
	return data;
}


