/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */
#include "serveur.h"

struct_partagee* memoire;

Joueur clients[NOMBRE_JOUEURS_MAX];
int nb_client = 0;

//Joueur* inscrits[NOMBRE_JOUEURS_MAX];
//int nb_joueurs = 0;

int timer_inscription_ecoule = FALSE;
int partie_en_cours = FALSE;
int distribution_paquet = FALSE;

int manche = 0;
int nb_cartes_par_joueur = 0;
int joueur_en_cours = 0;
Color couleur_tour;
Carte* pli_en_cours[NOMBRE_JOUEURS_MAX];
//int taille_pli_en_cours;

int end = FALSE;
int annule = FALSE;


int main(int argc, char** argv){
	int port;
	if(argc < 2 || argc > 3){
		printf("Usage : %s port [stderr]\n",argv[0]);
		return 1;
	}else if((port = atoi(argv[1])) <= 0){
		printf("Le port est invalide\n");
		return 1;
	}

	FILE* err;
	if(argc == 3){
		err = fopen(argv[2],"w");
		if(err==NULL) {
			fprintf(stderr,"Impossible d'ouvir le fichier %s : stderr sera utilisé\n",argv[2]);
		}else{
			dup2(fileno(err), STDERR_FILENO);
		}
	}

	if(CheckForAnotherInstance()) { //Ne fonctionne pas actuellement
		fprintf(stderr,"Une autre instance du serveur est déjà en exécution\n");
		fclose(err);
	    return 1;
	}

	srand(time(NULL));

	struct sigaction timer;
    timer.sa_handler = &handle_timer;
    sigemptyset(&(timer.sa_mask));
    timer.sa_flags = 0;

    SYS(sigaction(SIGALRM, &timer, NULL));

    int ma_socket = create_server_socket(port);

	while(TRUE){
		alarm(0);
		memoire = init_memoire();

		Carte c1 = {3,PIQUE};
		Carte c2 = {9,PAYOO};
		memoire->pli_en_cours[0] = &c1;
		memoire->pli_en_cours[1] = &c2;
		memoire->taille_pli_en_cours = 2;


		//Initialisation des variables pour la partie
		nb_client = 0;
		memoire->nb_joueurs = 0;

		timer_inscription_ecoule = FALSE;
		partie_en_cours = FALSE;

		manche = 0;
		nb_cartes_par_joueur = 0;

		memoire->taille_pli_en_cours = 0;

		end = FALSE;
		annule = FALSE;

		while(!end){
			int i;
			fd_set set;
			int fds[NOMBRE_JOUEURS_MAX];
			for(i = 0 ; i < nb_client ; i++){
				fds[i] = clients[i].fd;
			}
			int activity = attendre_message(ma_socket,fds,nb_client,&set);
			if(activity == 0){
				if(!partie_en_cours)
					continue;
				end = TRUE; //temps écoulé
				annule = TRUE;
				fprintf(stderr,"inactivité des clients\n");

				break;
			}else if(activity < 0){
				if (errno == EINTR)
            		continue; //SIGALRM
				fprintf(stderr,"erreur select\n");
				return 2;
			}

			// Nouvelle connexion d'un user
			if (FD_ISSET(ma_socket, &set)) {
				int nouveau_client_fd;
				struct sockaddr_in address;
				int addrlen = sizeof(address);
				SYS((nouveau_client_fd = accept(ma_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen)));

				if(nb_client >= NOMBRE_JOUEURS_MAX){
					Message ko;
					ko.type = CONNECTION_FULL;
					strcpy(ko.data.message, "Aucune place disponible\0");
					envoyer_message(nouveau_client_fd,ko);
					close(nouveau_client_fd);
					fprintf(stderr, "Un client a essayé de se connecter mais il n'y avais plus de connections disponnible\n");
				}else{
					// ajouter le nouveau user
					Joueur nouveau_client;
					nouveau_client.fd = nouveau_client_fd;
					nouveau_client.send_ecart = FALSE;
					strcpy(nouveau_client.nom,"\0");
					memcpy(&clients[nb_client], &nouveau_client, sizeof(Joueur));
					nb_client++;
				}
			}
			for(i = 0 ; i < nb_client ; i++){
				if (FD_ISSET(clients[i].fd, &set)) {
					Message msg = lire_message(clients[i].fd);
					handle_message(&clients[i],msg);
				}
			}
		}
		close_all();
		printf("Fin de la partie (%s)\n",annule?"annule":"ok");
	}
	fclose(err);
	return 0;
}

void handle_message(Joueur* client, Message msg){
	Message resp;
	int i;
	switch(msg.type){
		case INSCRIPTION:
			if(partie_en_cours){
				resp.type = INSCRIPTION_KO;
				strcpy(resp.data.message, "Une partie est en cours actuellement\0");
				envoyer_message(client->fd,resp);
			}else{
				strcpy(client->nom, msg.data.message);
				memoire->joueurs[memoire->nb_joueurs] = client;
				printf("Inscription : %s\n", client->nom);
				memoire->nb_joueurs++;
				if(memoire->nb_joueurs == 1){
					alarm(30);
				}
				resp.type = INSCRIPTION_OK;
				envoyer_message(client->fd,resp);
				if(memoire->nb_joueurs >= NOMBRE_JOUEURS_MAX || timer_inscription_ecoule){
					alarm(0);
					kill(getpid(),SIGALRM);
					//handle_timer(SIGALRM);
				}
			}
			return;
		case ANNULE:
			if(strlen(client->nom) == 0){
				fprintf(stderr,"Un joueur non-inscrit s'est déconnecté\n");
				int trouve = FALSE;
				//Un client s'est déconnecté
				for(i = 0 ; i < nb_client ; i++){
					if(clients[i].fd == client->fd){
						trouve = TRUE;
					}
					if(trouve){
						clients[i]=clients[i+1];
					}
				}
				nb_client--;
				close(client->fd);
			}else{
				fprintf(stderr,"%s s'est déconnecté\n",client->nom);
				end = TRUE;
				annule = TRUE;
			}
			return;
	}
	if(!partie_en_cours){
		bad_request(client,msg);
		return;
	}
	switch(msg.type){
		case ENVOI_PAQUET:
			if(nb_cartes_par_joueur > 0){
				bad_request(client,msg);
				return;
			}
			memcpy(client->ecart,msg.data.cartes,sizeof(Carte)*5);
			client->send_ecart = TRUE;
			if(check_ecart()){
				distribuer_paquet();
				demarrer_tour();
			}
			return;
		case JOUER_CARTE:
			if(client->fd != memoire->joueurs[joueur_en_cours]->fd){
				bad_request(client,msg);
				return;
			}
			if(memoire->taille_pli_en_cours == 0){
				couleur_tour = msg.data.cartes[0].couleur;
			}
			pli_en_cours[memoire->taille_pli_en_cours] = &msg.data.cartes[0];
			memoire->taille_pli_en_cours++;
			joueur_en_cours++;
			if(joueur_en_cours >= memoire->nb_joueurs){
				joueur_en_cours=0;
			}

			for(i = 0 ; i < memoire->nb_joueurs ; i++){ //Avertir tous les joueurs du nouveau pli en cours
				resp.type = AVERTIR_PLI_EN_COURS;
				envoyer_message(memoire->joueurs[i]->fd,resp);
			}

			if(memoire->taille_pli_en_cours >= memoire->nb_joueurs){ // cloture le tour
				cloturer_tour();
				if(nb_cartes_par_joueur>0)demarrer_tour();
				else{
					//cloturer manche
				}
			}
			return;
	}
}

void close_all(){
	int i;
	for(i = 0 ; i < nb_client ; i++){
		Message msg = {ANNULE};
		if(annule){
			envoyer_message(clients[i].fd,msg);
		}
		close(clients[i].fd);
	}
	//detacher(memoire);
}

void handle_timer(int signal){
    if(signal==SIGALRM){
    	timer_inscription_ecoule = TRUE;
        if(memoire->nb_joueurs>1){
        	demarrer_partie();
        }
    }
}

void demarrer_partie(){
	printf("Debut de la partie\n");
	partie_en_cours = TRUE;
	int i;

	for(i = 0 ; i < memoire->nb_joueurs ; i++){
		Message debut = {DEBUT_PARTIE};
		envoyer_message(memoire->joueurs[i]->fd,debut);
	}
	demarrer_manche();
}

void demarrer_manche(){
	manche++;
	int nb_cartes;
	Carte* cartes = paquet(&nb_cartes);
	const int NB_CARTES = nb_cartes;
	int i,j;

	for(i = 0 ; i < memoire->nb_joueurs ; i++){
		Carte main[30];
		for(j = 0 ; j < NB_CARTES/memoire->nb_joueurs ; j++){
			main[j] = getRandomCarte(cartes,&nb_cartes);
		}
		char nb_c[3];
		sprintf(nb_c, "%d\0", NB_CARTES/memoire->nb_joueurs);
		Message distribution = {DISTRIBUTION_CARTES,{nb_cartes,main}};
		envoyer_message(memoire->joueurs[i]->fd,distribution);
	}
	nb_cartes_par_joueur = 60/memoire->nb_joueurs;
	demarrer_tour();
}

void demarrer_tour(){
	demander_carte();
}

void cloturer_tour(){
	nb_cartes_par_joueur--;
	Message pli = {ENVOI_PLI};
	memcpy(pli.data.cartes,&pli_en_cours,sizeof(Carte)*memoire->taille_pli_en_cours);
	sprintf(pli.data.message, "%d\0", memoire->taille_pli_en_cours);
	int i = 0, j = joueur_en_cours;
	int max_carte = 0;
	int joueur_max;

	for( i = 0 ; i < memoire->nb_joueurs ; i++){
		if(j++ >= memoire->nb_joueurs) j = 0;
		if(pli_en_cours[i]->valeur > max_carte && pli_en_cours[i]->couleur == couleur_tour){
			max_carte = pli_en_cours[i]->valeur;
			joueur_max = j;
		}
	}
	envoyer_message(memoire->joueurs[joueur_max]->fd,pli);
	joueur_en_cours = joueur_max;
}

void demander_carte(){
	Joueur* c = memoire->joueurs[joueur_en_cours];
	Message demande_carte = {DEMANDER_CARTE};
	envoyer_message(c->fd,demande_carte);
}

int check_ecart(){
	int i;
	int ok = TRUE;
	for(i = 0 ; i < memoire->nb_joueurs ; i++){
		if(!memoire->joueurs[i]->send_ecart){
			ok=FALSE;
			break;
		}
	}
	return ok;
}

void distribuer_paquet(){
	int i;
	Joueur* c = memoire->joueurs[memoire->nb_joueurs-1];
	for(i = 0 ; i < memoire->nb_joueurs ; i++){
		Message distribution = {DISTRIBUTION_PAQUET};
		memcpy(&distribution.data.cartes,c->ecart,sizeof(Carte)*5);
		envoyer_message(memoire->joueurs[i]->fd,distribution);
		c = memoire->joueurs[i];
	}
}

void bad_request(Joueur* client,Message msg){
	fprintf(stderr,"Reception d'un message non-autorisé(%d)\n",msg.type);
	Message bad_request = {BAD_REQUEST};
	envoyer_message(client->fd,bad_request);
}

int attendre_message(int ma_socket, int* fds, int nb_fd, fd_set* set){
	int i = 0;
	int max_fd = ma_socket;

	struct timeval alive;
	alive.tv_sec = 30;
	alive.tv_usec = 0;

	FD_ZERO(set);
	FD_SET(ma_socket,set);

	for(i = 0 ; i < nb_fd ; i++){
		int fd = fds[i];
		if(fd > max_fd)max_fd = fd;
		FD_SET(fd,set);
	}

	int activity = select(max_fd+1,set,NULL,NULL,&alive);
	return activity;
}

int CheckForAnotherInstance(){ // Ne fonctionne pas actuellement
  int fd;
  struct flock fl;
  fd = open("/var/run/papayoo.pid", O_RDWR);
  if(fd == -1)
  {
    return FALSE;
  }

  fl.l_type   = F_WRLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
  fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
  fl.l_start  = 0;        /* Offset from l_whence         */
  fl.l_len    = 0;        /* length, 0 = to EOF           */
  fl.l_pid    = getpid(); /* our PID                      */
  // try to create a file lock

  if( fcntl(fd, F_SETLK, &fl) == -1){
    if( errno == EACCES || errno == EAGAIN){
      return TRUE;
    }
  }
  return FALSE;
}