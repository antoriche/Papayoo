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


int to_server_socket = -1;


int main ( int argc,char**argv ){

  if(argc!=2){
    fprintf(stderr,"Usage : %s ip_server ",argv[0]);
    exit(1);
  }

  Carte* cartes;
  int nbCartes;


 
  char buffer[512];
  int retval;
  fd_set set;
  struct timeval alive;
  alive.tv_sec = 30;
  alive.tv_usec = 0;

  /*
    Connexion au serveur
  */
  to_server_socket = connect_to_server(argv[1],30000);
  /*FD_ZERO(&set);
  FD_SET(0,&set);
  FD_SET(to_server_socket,&set);*/


  /*
    Inscription du joueur
  */
  char nom[51];
  int nbCharLus;

  printf("Entrez votre nom : \n");
  if((nbCharLus=read(0,nom,50))==-1){
    perror("erreur lecture");
    exit(1);
  }
  nom[nbCharLus-1]='\0';
 
  Message message = {INSCRIPTION};
  strcpy(message.message, nom);

  printf("Envoi du message : %s\n", message.message);
  write(to_server_socket,&message,sizeof(message));


  


  while(1){

    FD_ZERO(&set);
    FD_SET(0,&set);
    FD_SET(to_server_socket,&set);
    
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

  /* 
    fermeture de la connection 
  */
  shutdown(to_server_socket,2);
  close(to_server_socket);
  return 0;
}

void handle_message(Message message,Carte** cartes,int* nbCartes){
  Message resp;
  switch(message.type){
    case CONNECTION_FULL : 
      printf("Nombre max de connexions au serveur atteint.\n");
      exit(0);

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
      break;

    case DISTRIBUTION_PAQUET : 
      break;

    case DEMANDER_CARTE : 
      break;

    case AVERTIR_PLI_EN_COURS : 
      break;

    case ENVOI_PLI : 
      break;

    case COMPTER_POINTS : 
      break;

    case FIN_PARTIE : 
      printf("La partie actuelle est finie.\n");
      break;

  }
}