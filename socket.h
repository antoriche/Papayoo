#ifndef SOCKET_H_
#define SOCKET_H_

	int create_socket(int port);
	int attendre_message(int ma_socket, int* fds, int nb_fd, fd_set* joueurs);


#endif