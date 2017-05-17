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
#include "ipc.h"



int to_server_socket = -1;
Carte cartes[30];
int nbCartes;
int score;

int selection_paquet = FALSE; //indique que le client attend que l'on rentre la selection du paquet au clavier
int selection_carte = FALSE;
int taille_paquet = 0;
Carte mon_paquet[5];


int main ( int argc,char**argv ){

  int port;
  if(argc!=3){
    fprintf(stderr,"Usage : %s port ip_server\n",argv[0]);
    exit(1);
  }else if((port = atoi(argv[1])) <= 0){
    fprintf(stderr,"Le port est invalide\n");
    exit(1);
  }



 
  char buffer[512];
  int retval;
  fd_set set;
  struct timeval alive;
  alive.tv_sec = 30;
  alive.tv_usec = 0;

  /*
    Connexion au serveur
  */
  if((to_server_socket = connect_to_server(argv[2],port)) <0){
    printf("demande de connection ratee\n");
    exit(0);
  }

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
    FD_SET(STDIN_FILENO,&set);
    FD_SET(to_server_socket,&set);
    
    retval=select(to_server_socket+1,&set,NULL,NULL,&alive);
    if(retval==-1){
      perror("Erreur select");
    }
    else if(retval==0){
      
    }else{
      if(FD_ISSET(to_server_socket,&set)){
        Message m=lire_message(to_server_socket);
        handle_message(m);
      }
      if(FD_ISSET(STDIN_FILENO,&set)){
        char buffer[255];
        if(fgets(buffer,255,stdin) == NULL){
          //CTRL D
          Message resp = {ANNULE};
          envoyer_message(to_server_socket,resp);
          close(to_server_socket);
          exit(0);
        }
        handle_keyboard(buffer);
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

void handle_message(Message message){
  struct_partagee memoire;
  int i;
  Message resp;
  switch(message.type){
    case CONNECTION_FULL : 
      printf("Nombre max de connexions au serveur atteint.\n");
      
       
  
      exit(0);

    case INSCRIPTION_OK : 
      printf("Vous êtes bien inscrit!\n");
  
      //TEST ACCES MEMOIRE PARTAGEE
      /*struct_partagee memoire = lire_memoire();  
      Carte* ptr=memoire.pli_en_cours; 
      printf("Nombre de joueurs : %d\n",memoire.nb_joueurs);
      printf("Taille du pli : %d\n",memoire.taille_pli_en_cours);
      printf("Pli  : \n");
      int i = 0;
      for(i = 0 ; i < memoire.taille_pli_en_cours ; i++){
        printf("%s\n",carte2str(ptr[i]));
      }*/
      
      break;

    case INSCRIPTION_KO : 
      printf("Impossible de s'inscrire à la partie!\n");
      exit(1);

    case DEBUT_PARTIE :
      printf("La partie a commencé\n");
      struct_partagee memoire = lire_memoire();  
      printf("Liste des joueurs :\n");
      for( i = 0 ; i < memoire.nb_joueurs ; i++){
        printf("\t- %s\n",memoire.joueurs[i].nom);
      }
      break;

    case ANNULE :
      printf("La partie s'est terminee prematurement!\n");
      exit(1);

    case DISTRIBUTION_CARTES : 
      printf("Cartes distribuees\n");
      //cartes=message.data.cartes;
      memcpy(cartes,message.data.cartes,sizeof(Carte)*30);
      Carte* ptr=cartes;
      i=0;
      while(ptr->valeur!=CARTE_NULL && i<30){
        i++;
        ptr++;
      }
      nbCartes = i;
      afficher_cartes();
      printf("Veuillez choisir les 5 cartes à écarter.\n");
      
      taille_paquet=0;
      selection_paquet = TRUE;
      selection_carte = FALSE;
      
      break;

    case DISTRIBUTION_PAQUET : 
      printf("Paquet distribue\n");
      //cartes=message.data.cartes;
      memcpy(&cartes[nbCartes],message.data.cartes,sizeof(Carte)*5);
      nbCartes+=5;
      break;

    case DEMANDER_CARTE : 
      afficher_cartes();
      printf("Quelle carte voulez vous jouer? \n");
      selection_carte=TRUE;
      selection_paquet = FALSE;
      break;

    case AVERTIR_PLI_EN_COURS : 
      clear();
      afficher_pli_en_cours();
      break;

    case ENVOI_PLI : 
      recevoir_pli(message);
      break;

    case COMPTER_POINTS : 
      envoyer_points();

      break;

    case FIN_PARTIE : 
      printf("La partie actuelle est finie.\n");
      exit(0);

  }
}

void envoyer_points(){
  Message message={ENVOI_POINTS};
  sprintf(message.data.message,"%d\0",score);
  envoyer_message(to_server_socket,message);
}
void recevoir_pli(Message message){
  printf("Vous avez gagne ce pli! \n");
  struct_partagee memoire=lire_memoire();
  Color couleur_tour=memoire.couleur_tour;
  Carte* ptr= message.data.cartes;
  while(ptr->valeur!=CARTE_NULL){
    if(ptr->valeur==memoire.papayoo.valeur && ptr->couleur==memoire.papayoo.couleur) {
      score+=40;
    }else if(ptr->couleur==PAYOO){
      score+=ptr->valeur;
    }
      ptr++;
  }
}

void handle_keyboard(char* msg){
  if(selection_paquet){
    envoyer_paquet(msg);
  }if(selection_carte){
    int carte_id = atoi(msg)-1;
    choisir_carte_a_jouer(carte_id);
  }
}

void envoyer_paquet(char* msg){
  clear();
  int carte_id = atoi(msg)-1;
    if(carte_id < 0 || carte_id >= nbCartes){ // se protege contre les cartes non valides
      fprintf(stderr,"carte non valide\n");
      return;
    }

    mon_paquet[taille_paquet++] = cartes[carte_id]; // on ajoute la carte au paquet
    cartes[carte_id] = cartes[--nbCartes]; // et on la retire de notre main
    if(taille_paquet >= 5){
      Message m = {ENVOI_PAQUET};
      //m.data.cartes=mon_paquet;
      memcpy(m.data.cartes,mon_paquet,sizeof(Carte)*5);
      envoyer_message(to_server_socket,m);
      printf("Paquet envoye\n");
      selection_paquet = FALSE;
    }else{
      afficher_cartes();
      printf("-------------------\n");
      carte2str(mon_paquet[taille_paquet-1]) ;
      printf(" a été ajouté au paquet\n");
      printf("Carte suivante : \n");
    }

}


 void choisir_carte_a_jouer(int carte_id){
  Message message={JOUER_CARTE};
  struct_partagee memoire=lire_memoire();
  Color couleur_tour=memoire.couleur_tour;
  if(carte_id < 0 || carte_id >= nbCartes){ // se protege contre les cartes non valides
      fprintf(stderr,"carte non valide\n");
      return;
  }
  if(memoire.taille_pli_en_cours>0 && cartes[carte_id].couleur!=couleur_tour){
    Carte* ptr=cartes;
    int i;
    for(i=0;i<nbCartes;i++){
      if(ptr->couleur==couleur_tour){
        fprintf(stderr,"carte de la mauvaise couleur, choisissez un autre.\n");
        return;
      }
      ptr++;
    }
  }
  memcpy(message.data.cartes,&cartes[carte_id],sizeof(Carte));
  cartes[carte_id]= cartes[--nbCartes];
  envoyer_message(to_server_socket,message);
  printf("Carte jouee : ");
  carte2str(message.data.cartes[0]);
  printf("\n");
  selection_carte = FALSE;

}

void afficher_pli_en_cours(){
  struct_partagee memoire = lire_memoire();
  Color c=memoire.couleur_tour;
  
  printf("Couleur du pli en cours : ");
  if(memoire.taille_pli_en_cours!=0)
    afficher(c);

  int i;
  for(i=0;i<memoire.taille_pli_en_cours;i++){
    carte2str(memoire.pli_en_cours[i]);
    printf("\n");
  }
  printf("\n");
}

void afficher_cartes(){
  printf("Votre  score actuel : %d\n",score);
  struct_partagee memoire = lire_memoire();
  printf("Papayoo : ");
  carte2str(memoire.papayoo);
  printf("\n");
  int i = 0;
  for(i = 0 ; i < nbCartes ; i++){
    printf("%d) ",i+1);
    carte2str(cartes[i]);
    printf("\n");
  }
}

void clear(){
  system("clear");
}