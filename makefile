#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################

params = -w

all : server client clean

clean :
	rm *.o

# SERVER

server : server.o memoire.o socket.o message.o
	gcc $(params) -o server server.o memoire.o socket.o message.o

server.o : server.c server.h
	gcc $(params) -c server.c

# CLIENT

client : client.o memoire.o socket.o message.o
	gcc $(params) -o client client.o memoire.o socket.o message.o

client.o : client.c client.h
	gcc $(params) -c client.c


# PARTIE COMMUNE

memoire.o : memoire.c memoire.h
	gcc $(params) -c memoire.c

socket.o : socket.c socket.h
	gcc $(params) -c socket.c

message.o : message.c message.h
	gcc $(params) -c message.c