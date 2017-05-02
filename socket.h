#ifndef SOCKET_H_
#define SOCKET_H_
	
#define NOMBRE_JOUEURS_MAX 4

int create_server_socket(int port);
int connect_to_server(char* ip,int port);
int attendre_message(int ma_socket, int* fds, int nb_fd, fd_set* joueurs);


#endif