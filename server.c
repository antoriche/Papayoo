#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/select.h>

#include "message.h"
#include "server.h"
#include "socket.h"

Client clients[NOMBRE_JOUEURS_MAX];
int nb_client = 0;

Client* inscrits[NOMBRE_JOUEURS_MAX];
int nb_inscrit = 0;

int timer_inscription_ecoule = FALSE;
int partie_en_cours = FALSE;
int distribution_paquet = FALSE;

int manche = 0;
int nb_cartes = 0;
int joueur_en_cours = 0;

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

	srand(time(NULL));

	struct sigaction timer;
    timer.sa_handler = &handle_timer;
    sigemptyset(&(timer.sa_mask));
    timer.sa_flags = 0;

    SYS(sigaction(SIGALRM, &timer, NULL));

    int ma_socket = create_server_socket(port);

	while(TRUE){
		alarm(0);

		//Initialisation des variables pour la partie
		nb_client = 0;
		nb_inscrit = 0;

		timer_inscription_ecoule = FALSE;
		partie_en_cours = FALSE;

		manche = 0;
		nb_cartes = 0;

		end = FALSE;
		annule = FALSE;

		while(!end){
			//int s = select(max_fd+1,&joueurs,NULL,NULL,&alive);
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

				//printf("New connection , socket fd is %d\n" , client);

				if(nb_client >= NOMBRE_JOUEURS_MAX){
					//TODO : Envoyer message ?
					Message ko;
					ko.type = CONNECTION_FULL;
					strcpy(ko.message, "Aucune place disponible\0");
					envoyer_message(nouveau_client_fd,ko);
					close(nouveau_client_fd);
					fprintf(stderr, "Un client a essayé de se connecter mais il n'y avais plus de connections disponnible\n");
				}else{
					// ajouter le nouveau user
					Client nouveau_client;
					nouveau_client.fd = nouveau_client_fd;
					nouveau_client.send_ecart = FALSE;
					clients[nb_client] = nouveau_client;
					nb_client++;
				}
			}
			if (FD_ISSET(STDIN_FILENO, &set)) {
				//On a ecrit quelque chose au clavier
				char buffer[10];
				read(STDIN_FILENO,buffer,10);
				if(strcmp(buffer,"exit")){
					//fermeture propre du serveur
					fclose(err);
					return 0;
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
		printf("Fin de la partie\n");
	}
	fclose(err);
	return 0;
}

void handle_message(Client* client, Message msg){
	Message resp;
	int i;
	int n = -1;
	switch(msg.type){
		case INSCRIPTION:
			if(partie_en_cours){
				resp.type = INSCRIPTION_KO;
				//strcpy(resp.message, "Une partie est en cours actuellement\0");
				envoyer_message(client->fd,resp);
			}else{
				strcpy(client->nom, msg.message);
				inscrits[nb_inscrit] = client;
				printf("Inscription : %s\n", inscrits[nb_inscrit]->nom);
				nb_inscrit++;
				if(nb_inscrit == 1){
					alarm(30);
				}
				resp.type = INSCRIPTION_OK;
				envoyer_message(client->fd,resp);
				if(nb_inscrit >= NOMBRE_JOUEURS_MAX){
					alarm(0);
					kill(getpid(),SIGALRM);
					//handle_timer(SIGALRM);
				}
			}
			return;
		case ANNULE:
			//Un client s'est déconnecté
			for(i = 0 ; i < nb_client-1 ; i++){
				if(clients[i].fd == client->fd){
					n = i;
				}
				if(n<0){
					clients[i]=clients[i+1];
				}
			}
			nb_client--;
			if(strlen(client->nom) == 0){
				fprintf(stderr,"Un joueur non-inscrit s'est déconnecté\n");
			}else{
				fprintf(stderr,"%s s'est déconnecté\n",client->nom);
				end=TRUE;
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
			if(nb_cartes > 0){
				bad_request(client,msg);
				return;
			}
			memcpy(client->ecart,msg.cartes,sizeof(Carte)*5);
			client->send_ecart = TRUE;
			if(check_ecart()){
				distribuer_paquet();
				demarrer_tour();
			}
			return;
		case JOUER_CARTE:
			if(client->fd != inscrits[joueur_en_cours]->fd){
				bad_request(client,msg);
				return;
			}
			//TODO
			return;
	}
}

void close_all(){
	int i;
	for(i = 0 ; i < nb_client ; i++){
		Message msg = {ANNULE,NULL,NULL};
		if(annule)envoyer_message(clients[i].fd,msg);
		close(clients[i].fd);
	}
}

void handle_timer(int signal){
    if(signal==SIGALRM){
    	timer_inscription_ecoule = TRUE;
        if(nb_inscrit>1){
        	demarrer_partie();
        }
    }
}

void demarrer_partie(){
	printf("Debut de la partie\n");
	partie_en_cours = TRUE;
	int i;

	for(i = 0 ; i < nb_inscrit ; i++){
		Message debut = {DEBUT_PARTIE};
		envoyer_message(inscrits[i]->fd,debut);
	}
	demarrer_manche();
}

void demarrer_manche(){
	manche++;
	int nb_cartes;
	Carte* cartes = paquet(&nb_cartes);
	const int NB_CARTES = nb_cartes;
	int i,j;

	for(i = 0 ; i < nb_inscrit ; i++){
		Carte main[30];
		for(j = 0 ; j < NB_CARTES/nb_inscrit ; j++){
			main[j] = getRandomCarte(cartes,&nb_cartes);
		}
		char nb_cartes[3];
		sprintf(nb_cartes, "%d\0", NB_CARTES/nb_inscrit);
		Message distribution = {DISTRIBUTION_CARTES,nb_cartes,main};
		envoyer_message(inscrits[i]->fd,distribution);
	}
	demarrer_tour();
}

void demarrer_tour(){
	nb_cartes = 60/nb_inscrit;
	//premier_joueur = rand()%nb_inscrit;
	demander_carte();
}

void demander_carte(){
	Client* c = inscrits[joueur_en_cours];
	Message demande_carte = {DEMANDER_CARTE};
	envoyer_message(c->fd,demande_carte);
}

int check_ecart(){
	int i;
	int ok = TRUE;
	for(i = 0 ; i < nb_inscrit ; i++){
		if(!inscrits[i]->send_ecart){
			ok=FALSE;
			break;
		}
	}
	return ok;
}

void distribuer_paquet(){
	int i;
	Client* c = inscrits[nb_inscrit-1];
	for(i = 0 ; i < nb_inscrit ; i++){
		Message distribution = {DISTRIBUTION_PAQUET};
		//distribution.cartes = c->ecart;
		memcpy(&distribution.cartes,c->ecart,sizeof(Carte)*5);
		envoyer_message(inscrits[i]->fd,distribution);
		c = inscrits[i];
	}
}

void bad_request(Client* client,Message msg){
	fprintf(stderr,"Reception d'un message non-autorisé(%d)\n",msg.type);
	Message bad_request = {BAD_REQUEST};
	envoyer_message(client->fd,bad_request);
}