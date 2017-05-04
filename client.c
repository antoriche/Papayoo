#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "message.h"
#include "client.h"
#include "socket.h"

#define SERVEURNAME "127.0.0.1"
int to_server_socket = -1;


int main ( void ){

  Carte* cartes;
  int nbCartes;

  char buffer[512];
  int retval;
  fd_set set;
  struct timeval alive;
  alive.tv_sec = 30;
  alive.tv_usec = 0;


  to_server_socket = connect_to_server(SERVEURNAME,30000);
  FD_ZERO(&set);
  FD_SET(0,&set);
  FD_SET(to_server_socket,&set);


  Message message = {INSCRIPTION,"Coucou\0",NULL};


  printf("Envoi du message : %s\n", message.message);
  /*message.type=INSCRIPTION;
  char txt[256] = "coucou\0";
  message.message = txt;*/

  write(to_server_socket,&message,sizeof(message));

  printf("Lecture d'un message : \n");
  
  /*read(to_server_socket,&m,sizeof(Message));
  printf("%s\n",m.message);*/
  while(1){

    retval=select(to_server_socket+1,&set,NULL,NULL,&alive);
    if(retval==-1){
      perror("Erreur select");
    }
    else if(retval==0){

    }else{
      if(FD_ISSET(to_server_socket,&set)){
        Message m=lire_message(to_server_socket);
        handle_message(m,&cartes,&nbCartes);
      }
      
    }

  }
  //wait
  read(0,buffer,512);

  /* fermeture de la connection */
  shutdown(to_server_socket,2);
  close(to_server_socket);
  return 0;
}

void handle_message(Message message,Carte** cartes,int* nbCartes){

  Message resp;
  switch(message.type){
    case INSCRIPTION_OK : 
      printf("Vous êtes bien inscrit!\n");
      break;

    case INSCRIPTION_KO : 
      printf("Impossible de s'inscrire à la partie!\n");
      exit(1);

    case DEBUT_PARTIE :
      printf("La partie a commencé\n");
      break;

    case ANNULE :
      printf("La partie s'est terminee prematurement!\n");
      exit(1);

    case DISTRIBUTION_CARTES : 
      printf("Cartes distribuees\n");
      *cartes=message.cartes;
      *nbCartes=atoi(message.message);
  }
}