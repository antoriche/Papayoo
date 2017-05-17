#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################

params = -w -pthread 

all : serveur joueur clean

clean :
	rm *.o

# serveur

serveur : serveur.o ipc.o socket.o utils.o
	gcc $(params) -o serveur serveur.o ipc.o socket.o utils.o 

serveur.o : serveur.c serveur.h
	gcc $(params) -c serveur.c 

# joueur

joueur : joueur.o ipc.o socket.o utils.o
	gcc $(params) -o joueur joueur.o ipc.o socket.o utils.o


joueur.o : joueur.c joueur.h
	gcc $(params) -c joueur.c



# PARTIE COMMUNE

ipc.o : ipc.c ipc.h
	gcc $(params) -c ipc.c -lrt


socket.o : socket.c socket.h
	gcc $(params) -c socket.c 


#utils.o : utils.c utils.h
	#gcc $(params) -c utils.c 

utils.o : utils.c utils.h
	gcc $(params) -c utils.c
