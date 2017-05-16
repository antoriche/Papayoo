/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */
#include "socket.h"

int create_server_socket(int port){
	int ma_socket;
	struct sockaddr_in mon_address;
	bzero(&mon_address,sizeof(mon_address));
	mon_address.sin_port = htons(port);
	mon_address.sin_family = AF_INET;
	mon_address.sin_addr.s_addr = htonl(INADDR_ANY);
	/* creation de socket */
	if ((ma_socket = socket(AF_INET,SOCK_STREAM,0))== -1)
	{
	  perror("erreur socket ");
	  exit(0);
	}
	if((bind(ma_socket,(struct sockaddr *)&mon_address,sizeof(mon_address))) < 0){
		fprintf(stderr,"Impossible de démarrer le serveur sur le port %d ",port);
		exit(1);
	}

	/* ecoute sur la socket */
	if(listen(ma_socket,NOMBRE_JOUEURS_MAX) < 0){
		perror("erreur listen ");
		exit(1);
	}
	return ma_socket;
}

int connect_to_server(char* ip,int port){
	int to_server_socket = -1;
	char *server_name = ip;
	struct sockaddr_in serverSockAddr;
	struct hostent *serverHostEnt;
	long hostAddr;
	long status;
	
	bzero(&serverSockAddr,sizeof(serverSockAddr));
	hostAddr = inet_addr(ip);
	if ( (long)hostAddr != (long)-1)
		bcopy(&hostAddr,&serverSockAddr.sin_addr,sizeof(hostAddr));
	else{
		serverHostEnt = gethostbyname(ip);
		if (serverHostEnt == NULL){
			printf("gethost rate\n");
			exit(0);
		}
		bcopy(serverHostEnt->h_addr,&serverSockAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSockAddr.sin_port = htons(port);
	serverSockAddr.sin_family = AF_INET;
	/* creation de la socket */
	if ( (to_server_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		printf("creation socket client ratee\n");
		exit(0);
	}
	/* requete de connexion */
	if(connect( to_server_socket, (struct sockaddr *)&serverSockAddr,sizeof(serverSockAddr)) < 0 ){
		return -1;
	}
	return to_server_socket;
}