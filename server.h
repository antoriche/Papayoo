#ifndef SERVER_H_
#define SERVER_H_

typedef struct client{
	int fd;
	char nom[256];
	int send_ecart;
	Carte ecart[5];
} Client;

void handle_message(Client* client, Message msg);
void close_all();
void demarrer_partie();
void handle_timer(int signal);
int check_ecart();
void distribuer_paquet();
void bad_request(Client* client,Message msg);

#endif // SERVER_H_