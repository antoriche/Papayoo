#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/select.h>

#include "message.h"
#include "server.h"
#include "socket.h"

int fds[NOMBRE_JOUEURS_MAX];
int nb_fd = 0;

int partie_en_cours = FALSE;

int main(int argc, char** argv){
	int port;
	if(argc != 3){
		printf("Usage : %s port stderr\n",argv[0]);
		return 1;
	}else if((port = atoi(argv[1])) <= 0){
		printf("Le port est invalide\n");
		return 1;
	}

	
	int ma_socket = create_socket(port);

	//attendre_connexion(ma_socket,nouvelle_connexion);

	/*FD_ZERO(&joueurs);

	struct timeval alive;
	alive.tv_sec = 30;
	alive.tv_usec = 0;*/

	while(1){
		//int s = select(max_fd+1,&joueurs,NULL,NULL,&alive);
		int i;
		fd_set set;
		int activity = attendre_message(ma_socket,fds,nb_fd,&set);
		if(activity == 0){
			timeout(); //temps écoulé
			break;
		}else if(activity < 0){
			perror("erreur select");
			return 2;
		}

		// Nouvelle connexion d'un user
		if (FD_ISSET(ma_socket, &set)) 
		{
			int client;
			struct sockaddr_in address;
			int addrlen = sizeof(address);
			SYS((client = accept(ma_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen)));

			//printf("New connection , socket fd is %d\n" , client);

			if(partie_en_cours){
				Message ko = {INSCRIPTION_KO,NULL,NULL};
				envoyer_message(client,ko);
			}else{
				// ajouter le nouveau user
				fds[nb_fd] = client;
				nb_fd++;
			}
		}
		for(i = 0 ; i < nb_fd ; i++){
			if (FD_ISSET(fds[i], &set)) {
				Message msg = lire_message(fds[i]);
				if(msg.type == EMPTY_MSG){
					timeout();//ferme tous les clients
				}
				handle_message(fds[i],msg);
			}
		}
	}
	
	return 0;
}

void handle_message(int client, Message msg){
	switch(msg.type){
		case INSCRIPTION:
			printf("Demande d'INSCRIPTION du client n %d\n",client);
			break;

	}
}

void timeout(){
	printf("timeout\n");
	int i;
	for(i = 0 ; i < nb_fd ; i++){
		Message msg = {ANNULE,NULL,NULL};
		envoyer_message(fds[i],msg);
		close(fds[i]);
	}
	nb_fd = 0;
}