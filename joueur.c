/*
#############################################################
# Projet Unix - Papayoo
#
# LA Johnny
# RICHE Antonin
# SERIE 3
############################################################
 */

#include "joueur.h"
#include "memoire.h"



int to_server_socket = -1;


int main ( int argc,char**argv ){

  int port;
  if(argc!=3){
    fprintf(stderr,"Usage : %s port ip_server\n",argv[0]);
    exit(1);
  }else if((port = atoi(argv[1])) <= 0){
    fprintf(stderr,"Le port est invalide\n");
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
  to_server_socket = connect_to_server(argv[2],port);

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
  strcpy(message.data.message, nom);

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

        struct_partagee memoire = lire_memoire();
        /*
        struct_partagee donnees;    
        Carte c1 = {3,PIQUE};
        Carte c2 = {9,PAYOO};
        donnees.pli_en_cours[0] = &c1;
        donnees.pli_en_cours[1] = &c2;
*/
        Carte* ptr=memoire.pli_en_cours;
        
        printf("Nombre de joueurs : %d\n",memoire.nb_joueurs);
        printf("Taille du pli : %d\n",memoire.taille_pli_en_cours);
        
        printf("Pli en cours : \n");
        int i = 0;
        for(i = 0 ; i < memoire.taille_pli_en_cours ; i++){
          printf("%s\n",carte2str(ptr[i]));
        }
        
      }
      
    }

  }

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
      *cartes=message.data.cartes;
      *nbCartes=atoi(message.data.message);
      break;

    case DISTRIBUTION_PAQUET : 
      printf("Cartes distribuees\n");
      *cartes=message.data.cartes;
      *nbCartes=atoi(message.data.message);
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