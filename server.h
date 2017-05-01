#ifndef SERVER_H_
#define SERVER_H_
	
	#define NOMBRE_JOUEURS_MAX 4

	void handle_message(int client, Message msg);
	void timeout();

#endif // SERVER_H_