/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "message.h"
#include "socket.h"
#include "utils.h"

#define SYS(call) ((call) == -1) ? (perror(#call ": ERROR"),exit(1)) : 0

void handle_message(Message message);
void handle_keyboard(char* msg);
void choisir_carte_a_jouer(int carte_id);
void afficher_cartes();



#endif
