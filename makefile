#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################

params = -w

all : serveur joueur clean

clean :
	rm *.o

# serveur

serveur : serveur.o memoire.o socket.o message.o
	gcc $(params) -o serveur serveur.o memoire.o socket.o message.o

serveur.o : serveur.c serveur.h
	gcc $(params) -c serveur.c

# joueur

joueur : joueur.o memoire.o socket.o message.o
	gcc $(params) -o joueur joueur.o memoire.o socket.o message.o

joueur.o : joueur.c joueur.h
	gcc $(params) -c joueur.c


# PARTIE COMMUNE

memoire.o : memoire.c memoire.h
	gcc $(params) -c memoire.c

socket.o : socket.c socket.h
	gcc $(params) -c socket.c

message.o : message.c message.h
	gcc $(params) -c message.c