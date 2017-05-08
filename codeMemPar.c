//	code source du processus A
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <iostream>
using namespace std;

#define CLEF 12345 // je définis une clef au hasard

//	Je définis une structure quelconque qui comporte un entier et un double.
typedef struct
{
	int a;
	double b;
}structure_partagee;

int main()
{
	//	variable quelconque qui me sers pour un compteur plus bas...
	int i = 0;

	int mem_ID; //	identificateur du segment de mémoire partagée associé à CLEF
	void* ptr_mem_partagee; //	pointeur sur l'adresse d'attachement du segment de mémoire partagée

	//	J'instancie une structure "structure_partagee" et je l'appelle Data.
	structure_partagee Data;

	if ((mem_ID = shmget(CLEF, sizeof(Data), 0666 | IPC_CREAT)) < 0)	//	je crée un nouveau segment mémoire de taille "taille de ma structure data" octets, avec des droits d'écriture et de lecture
	{
		perror("shmget");											//	et je m'assure que l'espace mémoire a été correctement créé
		exit(1);
	}

	if ((ptr_mem_partagee = shmat(mem_ID, NULL, 0)) == (void*) -1)	//	J'attache le segment de mémoire partagée identifié par mem_ID au segment de données du processus A dans une zone libre déterminée par le Système d'exploitation
	{
		perror("shmat");											//	et je m'assure que le segment de mémoire a été correctement attaché à mon processus
		exit(1);
	}
	
	//	J'alloue des valeurs aux variables de ma structure
	Data.a = 2;
	Data.b = 2.6544;

	//	je mets à jour ces valeurs en mémoire partagée. ptr_mem_partagee est un pointeur de void. Je le caste pour qu'il devienne un pointeur de "structure_partagee" Et je vais écrire ma structure Data à l'adresse pointée par ce pointeur.

    	
    ((structure_partagee*)ptr_mem_partagee) = Data;


	
	//	je vais modifier en permanence le champ a de ma structure et le mettre à jour, le processus B lira la structure Data.
	while(1)
	{
		Data.a = i;

    		
    ((structure_partagee*)ptr_mem_partagee) = Data;


		i++;
		if(i == 100000000)	//	je remets à 0 de temps en temps...
			i = 0;
	}

	//	Une fois sortie de la boucle (bon OK là elle est infine), je détache mon segment mémoire de mon processus, et quand tous les processus en auront fait autant, ce segment mémoire sera détruit.
	shmdt(ptr_mem_partagee);

	//	je quitte le programme
	return 0;
}

//	code source du processus B
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
using namespace std;

#define CLEF 12345 // !!je définis la même clef que celle du processus A!!

//	Je crée la même structure que dans le programme A.
//	Les noms des variables n'ont rien à voir avec le programme A, seule la structure est importante.
typedef struct
{
	int c;
	double d;
}structure_partagee_B;

int main()
{
	//	je déclare des variables aptes à recevoir les variables de la structure "structure_partagee" définie dans le processus A
	int var1;
	double var2;

	int mem_ID_B; //	identificateur du segment de mémoire partagée associé à CLEF (là encore le nom de cette variable n'a rien à voir avec celle du programme A mais son contenu sera évidemment identique)
	void* ptr_mem_partagee_B; //	adresse d'attachement du segment de mémoire partagée (idem)

	//	J'instancie une structure "structure_partagee_B" et je l'appelle Data_B. Cela me sert uniquement à connaitre la taille de ma structure. Pour bien faire, il faudrait évidemment déclarer cette structure dans un .h qui serait inclu dans A et dans B avec la clef, de façon à garder la cohérence entre les 2 programmes
	structure_partagee_B Data_B;

	if ((mem_ID_B = shmget(CLEF, sizeof(Data_B), 0444)) < 0)	//	Je cherche le segment mémoire associé à CLEF et je récupère l'identificateur de ce segment mémoire... J'attribue des droits de lecture uniquement
	{
		perror("shmget");											//	et je m'assure que l'espace mémoire a été correctement créé
		exit(1);
	}
	if ((ptr_mem_partagee_B = shmat(mem_ID_B, NULL, 0)) == (void*) -1)	//	J'attache le segment de mémoire partagée identifié par mem_ID_B au segment de données du processus B dans une zone libre déterminée par le Système d'exploitation
	{
		perror("shmat");											//	et je m'assure que le segment de mémoire a été correctement attaché à mon processus
		exit(1);
	}

	//	j'affiche le contenu des variables inscrites par A dans la mémoire partagée
	while(1) {
		//	je caste ptr_mem_partagee_B pour qu'il devienne un pointeur de structure_partagee_B et j'affiche le champ c (ou d) de la structure pointée par ((structure_partagee_B*)ptr_mem_partagee_B)
		var1 = ((structure_partagee_B*)ptr_mem_partagee_B)->c;
		var2 = ((structure_partagee_B*)ptr_mem_partagee_B)->d;
		//	j'affiche le contenu des champs de la structure l'un à côté de l'autre, et je reviens au début de la ligne.
		cout << "data.a = " << var1 ;
		cout << " data.b = " << var2 << "\r";
	}

	//	Je détruis le segment (le segment n'est pas détruit tant qu'au moins un processus est lié au segment)
	shmdt(ptr_mem_partagee_B);

	//	je quitte le programme
	return 0;
}