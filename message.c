/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */
#include "message.h"



Message lire_message(int fd){
	Message message;
	int l = read(fd,&message, sizeof(Message));
	if(l <= 0){
		Message e = {ANNULE,{"\0"}};
		return e;
	}
	return message;
}

void envoyer_message(int client, Message message){
	SYS(write(client,&message,sizeof(Message)));
}
