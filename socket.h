/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

#include <errno.h>
#include <netdb.h>

#define NOMBRE_JOUEURS_MAX 4

int create_server_socket(int port);
int connect_to_server(char* ip,int port);


#endif