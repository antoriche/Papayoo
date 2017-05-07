/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */
#ifndef SERVER_H_
#define SERVER_H_

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
#include "socket.h"

typedef struct client{
	int fd;
	char nom[256];
	int send_ecart;
	Carte ecart[5];
} Client;

void handle_message(Client* client, Message msg);
void close_all();
void demarrer_partie();
void demarrer_manche();
void demarrer_tour();
void handle_timer(int signal);
int check_ecart();
void distribuer_paquet();
void bad_request(Client* client,Message msg);
int attendre_message(int ma_socket, int* fds, int nb_fd, fd_set* set);

#endif // SERVER_H_