#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "socket.h"

int create_socket(int port){
	int ma_socket;
	struct sockaddr_in mon_address;
	bzero(&mon_address,sizeof(mon_address));
	mon_address.sin_port = htons(port);
	mon_address.sin_family = AF_INET;
	mon_address.sin_addr.s_addr = htonl(INADDR_ANY);
	/* creation de socket */
	if ((ma_socket = socket(AF_INET,SOCK_STREAM,0))== -1)
	{
	  printf("la creation rate\n");
	  exit(0);
	}
	//signal(SIGINT,fin);
	/* bind serveur - socket */
	bind(ma_socket,(struct sockaddr *)&mon_address,sizeof(mon_address));

	/* ecoute sur la socket */
	listen(ma_socket,4);
	return ma_socket;
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