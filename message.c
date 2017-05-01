#include "message.h"

Message lire_message(int fd){
	Message message;
	int l = read(fd,&message, sizeof(Message));
	if(l <= 0){
		Message e = {EMPTY_MSG,"\0"};
		return e;
	}
	return message;
}

void envoyer_message(int client, Message message){
	SYS(write(client,&message,sizeof(Message)));
}

/*void envoyer_message(int client, int type, char* message){
	Message message = {type,message,NULL};
	envoyer_message(client,message);
}
void envoyer_message(int client, int type, Carte cartes[30]){
	Message message = {type,NULL,cartes};
	envoyer_message(client,message);
}*/