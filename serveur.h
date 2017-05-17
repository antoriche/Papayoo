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
#include <sys/file.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>

#include "message.h"
#include "ipc.h"
#include "socket.h"
#include "utils.h"

void handle_message(Joueur* joueur, Message msg);
void close_all_connections();
void close_server();
void demarrer_partie();
void demarrer_manche();
void demarrer_tour();
void cloturer_tour();
void handle_timer(int signal);
int check_ecart();
void distribuer_paquet();
void bad_request(Joueur* Joueur,Message msg);
int attendre_message(int ma_socket, int* fds, int nb_fd, fd_set* set);
int checkForAnotherInstance();
int check_score();
void cloturer_manche();

#endif // SERVER_H_