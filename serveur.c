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

#define TIMEOUT_INSCRIPTION 10

#define TIMEOUT_RESPONSE 60

#define NB_MANCHES 3
#define NB_PLI_MAX 3

struct_partagee memoire;

Joueur clients[NOMBRE_JOUEURS_MAX];
int nb_clients = 0;

//Joueur* inscrits[NOMBRE_JOUEURS_MAX];
//int nb_joueurs = 0;

int timer_inscription_ecoule = FALSE;
int partie_en_cours = FALSE;
int distribution_paquet = FALSE;

int manche = 0;
int nb_cartes_par_joueur = 0;
int nb_cartes_par_joueur_initial = 0;
int joueur_en_cours = 0;
//Color couleur_tour;
//Carte* pli_en_cours[NOMBRE_JOUEURS_MAX];
//int taille_pli_en_cours;

int end = FALSE;
int annule = FALSE;

int ma_socket;

FILE* err;

int main(int argc, char** argv){
	srand(time(NULL));

	
	int port;
	if(argc < 2 || argc > 3){
		printf("Usage : %s port [stderr]\n",argv[0]);
		return 1;
	}else if((port = atoi(argv[1])) <= 0){
		printf("Le port est invalide\n");
		return 1;
	}

	if(argc == 3){
		err = fopen(argv[2],"w");
		if(err==NULL) {
			fprintf(stderr,"Impossible d'ouvir le fichier %s : stderr sera utilisé\n",argv[2]);
		}else{
			dup2(fileno(err), STDERR_FILENO);
		}
	}

	if(checkForAnotherInstance()){
		fprintf(stderr, "Une autre instance du serveur est déjà en cours\n");
		exit(1);
	}
	memoire.port_actuel = port;

	init_sem();

	struct sigaction timer;
    timer.sa_handler = &handle_timer;
    sigemptyset(&(timer.sa_mask));
    timer.sa_flags = 0;

    SYS(sigaction(SIGALRM, &timer, NULL));

    signal(SIGINT,close_server);

    ma_socket = create_server_socket(port);
	while(TRUE){
		alarm(0);

		//Initialisation des variables pour la partie
		nb_clients = 0;
		memoire.nb_joueurs = 0;
		memoire.taille_pli_en_cours = 0;
		memoire.papayoo.valeur = 7;

		timer_inscription_ecoule = FALSE;
		partie_en_cours = FALSE;

		manche = 0;
		nb_cartes_par_joueur = 0;
		nb_cartes_par_joueur_initial = 0;

		end = FALSE;
		annule = FALSE;

		while(!end){
			ecrire_memoire(memoire);
			int i,j;
			fd_set set;
			int fds[NOMBRE_JOUEURS_MAX];
			for(i = 0 ; i < nb_clients ; i++){
				fds[i] = clients[i].fd;
			}
			int activity = attendre_message(ma_socket,fds,nb_clients,&set);
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
				close_server();
			}

			// Nouvelle connexion d'un user
			if (FD_ISSET(ma_socket, &set)) {
				int nouveau_client_fd;
				struct sockaddr_in address;
				int addrlen = sizeof(address);
				SYS((nouveau_client_fd = accept(ma_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen)));

				if(nb_clients >= NOMBRE_JOUEURS_MAX){
					Message ko;
					ko.type = CONNECTION_FULL;
					strcpy(ko.data.message, "Aucune place disponible\0");
					envoyer_message(nouveau_client_fd,ko);
					close(nouveau_client_fd);
					//fprintf(stderr, "Un client a essayé de se connecter mais il n'y avais plus de connections disponnible\n");
				}else{
					// ajouter le nouveau user
					Joueur nouveau_client;
					nouveau_client.fd = nouveau_client_fd;
					nouveau_client.send_ecart = FALSE;
					strcpy(nouveau_client.nom,"\0");
					memcpy(&clients[nb_clients],&nouveau_client,sizeof(Joueur));
					nb_clients++;
					//printf("nouvelle connection\n");
				}
			}
			for(i = 0 ; i < nb_clients ; i++){
				if (FD_ISSET(clients[i].fd, &set)) {
					Message msg = lire_message(clients[i].fd);
					handle_message(&clients[i],msg);
					for(j = 0 ; j < nb_clients ; j++){
						if(clients[i].fd == memoire.joueurs[j].fd){
							memcpy(&memoire.joueurs[j],&clients[i],sizeof(Joueur));
							ecrire_memoire(memoire);
							break;
						}
					}
				}
			}
		}
		close_all_connections();
		printf("Fin de la partie (%s)\n",annule?"annule":"ok");
	}
	close_server();
	return 0;
}

void handle_message(Joueur* client, Message msg){
	Message resp;
	int i,j;
	int points;
	switch(msg.type){
		case INSCRIPTION:
			if(partie_en_cours){
				resp.type = INSCRIPTION_KO;
				strcpy(resp.data.message, "Une partie est en cours actuellement\0");
				envoyer_message(client->fd,resp);
			}else{
				strcpy(client->nom, msg.data.message);
				memoire.joueurs[memoire.nb_joueurs] = *client;
				printf("Inscription : %s\n", client->nom);
				memoire.nb_joueurs++;
				if(memoire.nb_joueurs == 1){
					alarm(TIMEOUT_INSCRIPTION);
				}
				resp.type = INSCRIPTION_OK;
				envoyer_message(client->fd,resp);
				if(memoire.nb_joueurs >= NOMBRE_JOUEURS_MAX || timer_inscription_ecoule){
					alarm(0);
					kill(getpid(),SIGALRM);
					//handle_timer(SIGALRM);
				}
				ecrire_memoire(memoire);
			}
			return;
		case ANNULE:
			if(strlen(client->nom) == 0){
				//fprintf(stderr,"Un joueur non-inscrit s'est déconnecté\n");
				int trouve = FALSE;
				//Un client s'est déconnecté
				for(i = 0 ; i < nb_clients ; i++){
					if(clients[i].fd == client->fd){
						trouve = TRUE;
					}
					if(trouve){
						clients[i]=clients[i+1];
					}
				}
				nb_clients--;
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
			memcpy(client->ecart,msg.data.cartes,sizeof(Carte)*5);
			client->send_ecart = TRUE;
			if(check_ecart()){
				distribuer_paquet();
				demarrer_tour();
			}
			return;
		case JOUER_CARTE:
			if(client->fd != memoire.joueurs[joueur_en_cours].fd){
				bad_request(client,msg);
				return;
			}
			if(memoire.taille_pli_en_cours == 0){
				memoire.couleur_tour = msg.data.cartes[0].couleur;
			}
			printf("Carte reçue\n");
			memoire.pli_en_cours[memoire.taille_pli_en_cours] = msg.data.cartes[0];
			memoire.taille_pli_en_cours++;
			joueur_en_cours++;
			ecrire_memoire(memoire);
			if(joueur_en_cours >= memoire.nb_joueurs){
				joueur_en_cours=0;
			}

			for(i = 0 ; i < memoire.nb_joueurs ; i++){ //Avertir tous les joueurs du nouveau pli en cours
				resp.type = AVERTIR_PLI_EN_COURS;
				envoyer_message(memoire.joueurs[i].fd,resp);
			}

			if(memoire.taille_pli_en_cours >= memoire.nb_joueurs){ // cloture le tour
				cloturer_tour();
				if(nb_cartes_par_joueur>nb_cartes_par_joueur_initial-NB_PLI_MAX)demarrer_tour();
				else{
					// attend les scores puis cloture la manche
					/*for(i = 0 ; i < memoire.nb_joueurs ; i++){
						memoire.joueurs[i].score_en_attente = TRUE;
						resp.type = COMPTER_POINTS;
						envoyer_message(memoire.joueurs[i].fd,resp);
					}*/
					for(i = 0 ; i < nb_clients ; i++){
						int inscrit = FALSE;
						for(j = 0 ; j < memoire.nb_joueurs ; j++){
							if(memoire.joueurs[j].fd == clients[i].fd){
								inscrit = TRUE;
							}
						}
						if(inscrit){
							clients[i].score_en_attente = TRUE;
							resp.type = COMPTER_POINTS;
							envoyer_message(memoire.joueurs[i].fd,resp);
						}
					}
				}
			}else{
				demander_carte();
			}
			return;
		case ENVOI_POINTS:
			points = atoi(msg.data.message);
			client->score = client->score+points;
			client->score_en_attente = FALSE;
			if(check_score()){
				cloturer_manche();
			}
			return;
	}
}

void cloturer_manche(){
	if(manche >= NB_MANCHES){
		//cloturer partie
		end = TRUE;
	}else{
		demarrer_manche();
		printf("LOLOL\n");
	}
}

void close_all_connections(){
	int i;
	for(i = 0 ; i < nb_clients ; i++){
		Message msg;
		msg.type = annule?ANNULE:FIN_PARTIE;
		envoyer_message(clients[i].fd,msg);
		close(clients[i].fd);
	}
	nb_clients = 0;
	memoire.nb_joueurs = 0;
}
void close_server(){
	printf("fermeture du serveur\n");
	annule = TRUE;
	close_all_connections();
	close(ma_socket);
	cloturer_memoire();
	if(err != NULL)fclose(err);
	exit(0);
}

void handle_timer(int signal){
    if(signal==SIGALRM){
    	timer_inscription_ecoule = TRUE;
        if(memoire.nb_joueurs>1){
        	demarrer_partie();
        }else{
        	annule = TRUE;
        	end = TRUE;
        	close_all_connections();
        }
    }
}

void demarrer_partie(){
	printf("Debut de la partie\n");
	partie_en_cours = TRUE;
	int i;

	for(i = 0 ; i < memoire.nb_joueurs ; i++){
		Message debut = {DEBUT_PARTIE};
		envoyer_message(memoire.joueurs[i].fd,debut);
	}
	demarrer_manche();
	
}

void demarrer_manche(){
	manche++;
	printf("debut de manche %d\n",manche);
	int nb_cartes;
	Carte* cartes = paquet(&nb_cartes);
	const int NB_CARTES_TOTAL = nb_cartes;
	int i,j;
	printf("QSFDFDSQFQZF1\n");
	memoire.papayoo.couleur = rand()%4;
	ecrire_memoire(memoire);
	printf("QSFDFDSQFQZF2\n");
	for(i = 0 ; i < memoire.nb_joueurs ; i++){
		Carte main[30];
		for(j = 0 ; j < NB_CARTES_TOTAL/memoire.nb_joueurs ; j++){
			main[j] = getRandomCarte(cartes,&nb_cartes);
		}
		for(j = NB_CARTES_TOTAL/memoire.nb_joueurs ; j < 30 ; j++){
			main[j].valeur = CARTE_NULL;
		}
		Message distribution = {DISTRIBUTION_CARTES};
		memcpy(distribution.data.cartes,main,sizeof(Carte)*30);
		envoyer_message(memoire.joueurs[i].fd,distribution);
		printf("QSFDFDSQFQZF3\n");
	}
	printf("QSFDFDSQFQZF4\n");
	nb_cartes_par_joueur = 60/memoire.nb_joueurs;
	nb_cartes_par_joueur_initial = 60/memoire.nb_joueurs;
	joueur_en_cours = rand()%memoire.nb_joueurs;
}

void demarrer_tour(){
	printf("debut de tour\n");
	demander_carte();
}

void cloturer_tour(){
	nb_cartes_par_joueur--;
	Message pli = {ENVOI_PLI};
	memcpy(pli.data.cartes,memoire.pli_en_cours,sizeof(Carte)*memoire.taille_pli_en_cours);
	int i = 0, j = joueur_en_cours;
	for(i = memoire.taille_pli_en_cours ; i < 30 ; i++){
		pli.data.cartes[i].valeur = CARTE_NULL;
	}
	int max_carte = 0;
	int joueur_max;

	for( i = 0 ; i < memoire.nb_joueurs ; i++){
		
		if(j >= memoire.nb_joueurs) j = 0;
		if(memoire.pli_en_cours[i].valeur > max_carte && memoire.pli_en_cours[i].couleur == memoire.couleur_tour){
			max_carte = memoire.pli_en_cours[i].valeur;
			joueur_max = j;
		}
		j++;
	}
	envoyer_message(memoire.joueurs[joueur_max].fd,pli);
	joueur_en_cours = joueur_max;
	memoire.taille_pli_en_cours = 0;
	ecrire_memoire(memoire);
}

void demander_carte(){
	Joueur* c = &memoire.joueurs[joueur_en_cours];
	Message demande_carte = {DEMANDER_CARTE};
	envoyer_message(c->fd,demande_carte);
}

int check_ecart(){
	int i,j;
	int ok = TRUE;
	for(i = 0 ; i < nb_clients ; i++){
		int inscrit = FALSE;
		for(j = 0 ; j < memoire.nb_joueurs ; j++){
			if(memoire.joueurs[j].fd == clients[i].fd){
				inscrit = TRUE;
			}
		}
		if(!clients[i].send_ecart && inscrit){
			ok=FALSE;
			break;
		}
	}
	return ok;
}

/*
*	renvoi true si tous les joueurs ont envoyé leurs score
*/
int check_score(){
	int i,j;
	int ok = TRUE;
	for(i = 0 ; i < nb_clients ; i++){
		int inscrit = FALSE;
		for(j = 0 ; j < memoire.nb_joueurs ; j++){
			if(memoire.joueurs[j].fd == clients[i].fd){
				inscrit = TRUE;
			}
		}
		if(clients[i].score_en_attente && inscrit){
			ok=FALSE;
			break;
		}
	}
	return ok;
}

void distribuer_paquet(){
	int i,j;
	Joueur* c;
	for( i = nb_clients ; i >= 0 ; i--){ // on trouve le dernier client inscrit
		if(strlen(clients[i].nom) > 0){
			c = &clients[i];
			break;
		}
	}

	for(i = 0 ; i < nb_clients ; i++){
		int inscrit = FALSE;
		for(j = 0 ; j < memoire.nb_joueurs ; j++){
			if(memoire.joueurs[j].fd == clients[i].fd){
				inscrit = TRUE;
			}
		}
		if(inscrit){
			Message distribution = {DISTRIBUTION_PAQUET};
			memcpy(distribution.data.cartes,c->ecart,sizeof(Carte)*5);
			for( j = 5 ; j < 30 ; j++){
				distribution.data.cartes[j].valeur = CARTE_NULL;
			}
			envoyer_message(clients[i].fd,distribution);
			c = &clients[i];
		}
	}
	for(i = 0 ; i < nb_clients ; i++){
		clients[i].send_ecart = FALSE;
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
	alive.tv_sec = TIMEOUT_RESPONSE;
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

/*
*	Renvoi TRUE si une autre instance du serveur est en cours
*/
int checkForAnotherInstance(){
	memoire = lire_memoire();
	if(memoire.memoire_valide){
		//verifier en se connectant au port du serveur
		int resp;
		if((resp = connect_to_server("localhost",memoire.port_actuel)) < 0){
			cloturer_memoire();
			return 0;
		}
		close(resp);
		return 1;
	}else{
		return 0;
	}
}