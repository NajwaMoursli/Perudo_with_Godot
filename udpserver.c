// Server side implementation of an UDP client-server model
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 4242
#define MAXLINE 1024

struct _client
{
  char ipAddress[40]; // adresse ip du client
  int port;           // numéro de port du client
  char name[40];      // nom du joueur
  int nbrDes;         // nombre de dés du joueur
  int direction;
} udpClients[4];

int nbrClients = 0; // le nombre de joueur encore en liste, sera initialise à 4
                    // au fur et à mesure
int tableDes[6];    // un tableau des nombres de chaque face de tous les joueurs
int fsmServer = 0;

struct _pari
{
  struct _client
      *c;     // une structure client correspondant au joueur qui a fait le pari
  int nbrDes; // nombre de dés du pari en question
  int faceDes; // la face du dés du pari en question
};

void printClients()
{
  int i;

  for (i = 0; i < nbrClients; i++)
    printf("%d: %s %5.5d %s %d %d\n", i, udpClients[i].ipAddress,
           udpClients[i].port, udpClients[i].name, udpClients[i].nbrDes,
           udpClients[i].direction);
}

int findClientByName(char *name)
{

  for (int i = 0; i < nbrClients; i++)
    if (strcmp(udpClients[i].name, name) == 0)
      return i;
  return -1;
}

void sendMessage(struct _client cl, const char *message)
{ // envoie un message à un unique joueur
  /* socket: create the socket */
  int socketGodot;
  struct hostent *serverGodot;
  int serverlen;
  struct sockaddr_in serverGodotAddr;
  int n;

  socketGodot = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketGodot < 0)
  {
    // print("ERROR opening socketGodot\n");
    exit(EXIT_FAILURE);
  }

  /* gethostbyname: get the server's DNS entry */
  serverGodot = gethostbyname(cl.ipAddress);
  if (serverGodot == NULL)
  {
    fprintf(stderr, "ERROR, no such host as %s\n", cl.ipAddress);
    exit(EXIT_FAILURE);
  }

  /* build the server's Internet address */
  bzero((char *)&serverGodotAddr, sizeof(serverGodotAddr));
  serverGodotAddr.sin_family = AF_INET;
  bcopy((char *)serverGodot->h_addr, (char *)&serverGodotAddr.sin_addr.s_addr,
        serverGodot->h_length);
  serverGodotAddr.sin_port = htons(cl.port);

  /* send the message to the server */
  serverlen = sizeof(serverGodotAddr);
  // n = sendto(socketGodot, sendBuffer, strlen(sendBuffer), 0,
  n = sendto(socketGodot, message, strlen(message), 0,
             (struct sockaddr *)&serverGodotAddr, serverlen);
  if (n < 0)
  {
    // print("ERROR in sendto\n");
    exit(EXIT_FAILURE);
  }
  close(socketGodot);
}

int get_next_idClient(int enListe[4], int idClient)
{
  int id = (idClient + 1) % 4;
  while (enListe[id] == 0)
    id = (id + 1) % 4;
  return id;
}

void broadcast(struct _client udpClients[4], const char *message,
               int enListe[4], int idClient, int opt)
{ // broadcast un message aux joueurs
  // opt est une option prenant les valeur 1 ou zéro. Dans le cas où elle vaut
  // 1, on broadcast seulement aux joueurs encore en liste. Sinon on broadcast à
  // tout le monde
  if (opt == 1)
  {
    for (int i = 0; i < 3; ++i)
    {
      sendMessage(udpClients[(idClient + (i + 1)) % 4], message);
    }
  }
  else
  {
    for (int i = 0; i < 4; ++i)
    {
      sendMessage(udpClients[i], message);
    }
  }
}

void clean(int table[], int n)
{
  for (int i = 0; i < n; ++i)
  {
    table[i] = 0;
  }
}

// met à jour la table des dés globale (variable globale)
void init_table_des(int table_add[5])
{

  for (int i = 0; i < 5; ++i)
  {
    if (table_add[i] != 0)
      tableDes[table_add[i] - 1] += 1; //!!! table_add[i] plutot ?
  }
}

// renvoie le nombre total de tous les des de tous les joueurs
int totalDes(struct _client udpClients[4])
{
  int total = 0;
  for (int i = 0; i < nbrClients; ++i)
    total += udpClients[i].nbrDes;
  return total;
}

// renvoie l'id de l'éméteur du pari
int get_id_parieur(int enListe[4], int idClient)
{
  int id = (idClient - 1) % 4;
  while (enListe[id] == 0)
    id = (id - 1) % 4;
  // print("%d\n", id);
  return id;
}

char get_direction(int idClient)
{
  if (idClient == 0)
    return 'N';
  if (idClient == 1)
    return 'E';
  if (idClient == 2)
    return 'S';
  if (idClient == 3)
    return 'W';
}

// test le nombre de dés du parieur et retourne 1 si un des deux a perdu sinon
// zéro
int perduPasPerdu_parieur(struct _pari *p, struct _client udpClients[4],
                          int resultat, int enListe[4], int idClient)
{
  char update[256];
  if (resultat == 1 && p->c->nbrDes == 0)
  { // cest quoi resultat
    sprintf(update, "Q %d %s a perdu la partie !", p->c->direction, p->c->name);
    // print("%s\n", update);
    broadcast(udpClients, update, enListe, idClient, 0);
    nbrClients--;
    enListe[idClient] = 0;
    return 1;
  }
  return 0;
}

// test le nombre de dés du joueur et retourne 1 si un des deux a perdu sinon
// zéro
int perduPasPerdu_joueur(struct _client *cl, struct _client udpClients[4],
                         int resultat, int enListe[4], int idClient)
{
  char update[256];
  if (resultat == 0 && cl->nbrDes == 0)
  {
    sprintf(update, "Q %d %s a perdu la partie !", cl->direction, cl->name);
    // print("%s\n", update);
    broadcast(udpClients, update, enListe, idClient, 0);
    nbrClients--;
    enListe[idClient] = 0;
    return 1;
  }
  return 0;
}

/* lorsqu'un joueur pense que le pari du joueur précedent est érroné le serveur
vérifie le pari grâce à la fonction suivante */
int verification_menteur(struct _pari *p, struct _client *cl,
                         struct _client udpClients[4], int enListe[4],
                         int idClient, int valDes[24], int lock_pacifico)
{
  int tmp =
      tableDes[(p->faceDes) -
               1]; // correspond au nombre de dés du dé parié dans tout le jeu
  int paco;
  if (p->faceDes != 0 ||
      lock_pacifico == 0) // si la face choisie est pas un paco alors ils faut
                          // les ajouter pour le décompte
    paco = tableDes[0];
  else
    paco = 0;
  int nbr = p->nbrDes; // correspond au nombre de dés du numéro de dés parié
  int resultat;
  // const char anonce[32] = "Vous avez perdu un dé";
  const char perdu[32] = "Vous avez perdu un dé";
  const char gagne[32] = "Vous avez bien prédit";
  char anonce_parieur[256];
  char anonce_client[256];

  // on dévoile à tout le monde les dés
  char msg[256];
  sprintf(msg,
          "R %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
          valDes[0], valDes[1], valDes[2], valDes[3], valDes[4], valDes[5],
          valDes[6], valDes[7], valDes[8], valDes[9], valDes[10], valDes[11],
          valDes[12], valDes[13], valDes[14], valDes[15], valDes[16],
          valDes[17], valDes[18], valDes[19]);
  broadcast(udpClients, msg, enListe, idClient,
            0); // on broadcast à tous les joueurs quel était le nombre de dés
                // réel du pari

  // le parieur a raison et le menteur a tort
  if (nbr <= tmp + paco)
  { // Si le nombre de dés est inférieur ou égale le joueur qui dit menteur perd
    // un dés
    cl->nbrDes--;
    // print("nb de des du douteur %d\n", cl->nbrDes);
    resultat = 0; // c'est le joueur qui dit menteur qui perd un dé
    if (perduPasPerdu_joueur(cl, udpClients, resultat, enListe, idClient) == 0)
    {
      sprintf(anonce_client, "L %d %s\n", cl->direction, perdu);
      sprintf(anonce_parieur, "W %d %s\n", p->c->direction, gagne);
      // print("%s\n", anonce_client);
      // print("%s\n", anonce_parieur);
      broadcast(udpClients, anonce_client, enListe, idClient, 0);
      broadcast(udpClients, anonce_parieur, enListe, idClient, 0);
    }
    else
    {
      sprintf(anonce_parieur, "W %d %s\n", p->c->direction, gagne);
      // print("%s\n", anonce_parieur);
      broadcast(udpClients, anonce_parieur, enListe, idClient, 0);
    }
  }
  // le menteur a raison et le parieur a tort
  else
  { // Sinon c'est le joueur qui a fait le pari qui en perd un dé
    p->c->nbrDes--;
    // print("nb de des du parieur %d\n", p->c->nbrDes);
    resultat = 1; // c'est le joueur qui parie qui perd un dé
    if (perduPasPerdu_parieur(p, udpClients, resultat, enListe, idClient) == 0)
    {
      sprintf(anonce_client, "W %d %s\n", cl->direction, gagne);
      sprintf(anonce_parieur, "L %d %s\n", p->c->direction, perdu);
      // print("%s\n", anonce_client);
      // print("%s\n", anonce_parieur);
      broadcast(udpClients, anonce_parieur, enListe, idClient, 0);
      broadcast(udpClients, anonce_client, enListe, idClient, 0);
    }
    else
    {
      sprintf(anonce_client, "W %d %s\n", p->c->direction, gagne);
      // print("%s\n", anonce_client);
      broadcast(udpClients, anonce_client, enListe, idClient, 0);
    }
  }
  return resultat;
}

// permet de savoir si on est au tour suivant en pacifico :
int pacifico(int perdant, int idClient, struct _client udpClients[4],
             int enListe[4])
{
  char msg_pacifico_client[128]; //
  char msg_pacifico_parieur[128];
  int idParieur = get_id_parieur(enListe, idClient);
  if (perdant == 0 && udpClients[idClient].nbrDes == 1)
  { // pour le Palifico !!
    sprintf(msg_pacifico_client, "O %d on fait un tour de pacifico pour %s\n",
            idClient, udpClients[idClient].name);
    broadcast(udpClients, msg_pacifico_client, enListe, idClient, 0);
    return 2;
  }
  if (perdant == 1 && udpClients[idParieur].nbrDes == 1)
  { // pour le Palifico !!
    sprintf(msg_pacifico_parieur, "O %d on fait un tour de pacifico pour %s\n",
            idParieur, udpClients[idParieur].name);
    broadcast(udpClients, msg_pacifico_parieur, enListe, idClient, 0);
    return 1;
  }
  return 0;
}

// Driver code
int main()
{
  srand(time(NULL));
  int sockfd;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr, cliaddr;
  char com;
  char clientIpAddress[256], clientName[256];
  int clientPort, clientdirection, id;
  int idClient, pari_faceDes, pari_nbrDes;
  char reply[256];
  int D[5]; // correspond au tableau où on stock les nombres de faces du tirage
            // d'un joueur
  int enListe[4] = {1, 1, 1,
                    1}; // tableau des joueurs encore en liste si valeur vaut 1
  struct _pari *p_tmp = malloc(sizeof(struct _pari));
  // c'est le dernier pari proposé, il est mis à jour dès
  // qu'un joueur propose un nouveau pari
  char message[128], requete[128], message_fin[128], anonce_pacifico[128],
      anonce_tour[128];
  int valDes[20]; // les vraies valeurs de tous les dés de tous les joueurs
  int tirage_recu = 0;
  int lock_pacifico = 0;
  int id_pacifico;
  int idPerdant = -1;
  int idPremierJoueur;
  int permier_tour = 1;
  char msg_mensonge[128];
  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Server is on localhost:%d\n", PORT);

  int n;
  unsigned int len;

  len = sizeof(cliaddr); // len is value/resuslt

  clean(tableDes, 6); // On initialise tableDes à zéro
  printf("tabDes : %d %d %d %d %d %d\n", tableDes[0], tableDes[1], tableDes[2],
         tableDes[3], tableDes[4], tableDes[5]);
  clean(valDes, 20);
  printf("valDes %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         valDes[0], valDes[1], valDes[2], valDes[3], valDes[4], valDes[5],
         valDes[6], valDes[7], valDes[8], valDes[9], valDes[10], valDes[11],
         valDes[12], valDes[13], valDes[14], valDes[15], valDes[16], valDes[17],
         valDes[18], valDes[19]);

  while (1)
  {
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    // printf("n=%d\n",n);
    buffer[n] = '\0';

    if (fsmServer == 0)
    {
      switch (buffer[0])
      {
      case 'C': // on attend les connections de tout le monde
        sscanf(buffer, "%c %s %d %s %d\n", &com, clientIpAddress, &clientPort,
               clientName, &clientdirection);
        printf("COM=%c ipAddress=%s port=%d name=%s\n", com, clientIpAddress,
               clientPort, clientName);
        // print("nbClients = %d\n", nbrClients);
        // fsmServer==0 alors j'attends les connexions de tous les joueurs
        // implicitement un id est attribué au joueur d'après son ordre de
        // connexion : s'il est le premier à se connecter , il a 0, s'il est le
        // deuxième, il a 1 etc
        strcpy(udpClients[nbrClients].ipAddress, clientIpAddress);
        udpClients[nbrClients].port = clientPort;
        strcpy(udpClients[nbrClients].name, clientName);
        udpClients[nbrClients].direction = clientdirection;
        udpClients[nbrClients].nbrDes = 5;
        nbrClients++;
        printClients();

        // rechercher l'id du joueur qui vient de se connecter
        id = findClientByName(clientName);
        // printf("id=%d\n",id);

        // Envoyer un message broadcast pour communiquer a tout le monde la
        // liste des joueurs actuellement connectes
        sprintf(reply, "J %s %s %s %s\n", udpClients[0].name,
                udpClients[1].name, udpClients[2].name, udpClients[3].name);
        // print("liste joueurs : %s\n", reply);
        // printf("nbClients = %d\n", nbrClients);

        // broadcast(udpClients,reply,enListe, 0);
        for (int i = 0; i < nbrClients; ++i)
        {
          sendMessage(udpClients[i], reply);
        }

        if (nbrClients == 4)
        {
          // print("tous les joueurs sont arrivés\n");
          idPremierJoueur = rand() % 4;
          // print("indice premier joueur = %d\n", idPremierJoueur);
          sprintf(anonce_tour, "S %d\n",
                  idPremierJoueur); // S id : c'est au tour de id de jouer
          // print("%s\n", anonce_tour);
          broadcast(udpClients, anonce_tour, enListe, idClient,
                    0); // dit à tout le monde qui commence la 1ère manche
          for (int i = 0; i < 4; ++i)
            while (udpClients[i].direction != i)
              udpClients[i].direction++;
          fsmServer = 1;
        }
        else
        {
          // print("il y a encore de absents.\n");
        }
        break;
      }
    }
    else if (fsmServer == 1)
    {
      switch (buffer[0])
      {
      case 'M': // un des joueur dit menteur
        sprintf(msg_mensonge, "M %d pense qu'il n'y a pas %d %d\n", idClient,
                p_tmp->faceDes, p_tmp->nbrDes);
        broadcast(udpClients, msg_mensonge, enListe, idClient, 0);
        // print("%s\n",buffer);
        int perdant =
            verification_menteur(p_tmp, &udpClients[idClient], udpClients,
                                 enListe, idClient, valDes, lock_pacifico);
        // print("%d\n", perdant);
        if (perdant == 0) // c'est le cas où celui qui doute perd
          idPerdant = idClient;
        else // c'est le cas ou le parieur perd
          idPerdant = p_tmp->c->direction;
        clean(D, 5);
        clean(tableDes, 6);
        clean(valDes, 20);
        if (nbrClients == 1)
        { // cas de la fin de partie
          if (perdant == 0)
          { // le joueur courant à perdu
            int k = p_tmp->c->direction;
            sprintf(message_fin,
                    "F %d La partie est finie !\n %s remporte la victoire avec "
                    "%d dé(s) !\n",
                    k, udpClients[k].name, udpClients[k].nbrDes);
            broadcast(udpClients, message_fin, enListe, idClient, 0);
            // print("%s\n", message_fin);
          }
          else
          { // le parieur a perdu
            sprintf(message_fin,
                    "F %d La partie est finie !\n %s remporte la victoire avec "
                    "%d dé(s) !\n",
                    idClient, udpClients[idClient].name,
                    udpClients[idClient].nbrDes);
            broadcast(udpClients, message_fin, enListe, idClient, 0);
            // print("%s\n", message_fin);
          }
        }
        if (pacifico(perdant, idClient, udpClients, enListe) == 2)
        { // on vérifie le pacifico du joueur
          id_pacifico = idClient;
          lock_pacifico = 1;
          // print("pacifico joueur courant\n");
        }
        if (pacifico(perdant, idClient, udpClients, enListe) == 1)
        { // on vérifie le pacifico du parieur
          id_pacifico = p_tmp->c->direction;
          lock_pacifico = 1;
          // print("pacifico parieur\n");
        }
        if (lock_pacifico == 1)
        {
          sprintf(anonce_pacifico, "S %d\n", id_pacifico);
          broadcast(udpClients, anonce_pacifico, enListe, idClient, 0);
        }
        if (enListe[idPerdant] == 1)
        {
          sprintf(anonce_tour, "S %d\n", idPerdant);
          broadcast(udpClients, anonce_tour, enListe, idClient, 0);
        }
        else
        {
          sprintf(anonce_tour, "S %d\n", get_next_idClient(enListe, idPerdant));
          broadcast(udpClients, anonce_tour, enListe, idClient, 0);
        }
        // print("nbClients = %d\n", nbrClients);
        // print("arrivé à fin de case M\n");
        break;
      // case 'F': // cas de fin de parti
      //   sprintf(message, "F")
      //   broadcast(udpClients, message, enListe, 0);
      //  break;
      case 'P': // on reçoit le pari d'un joueur
        sscanf(buffer, "P %d %d %d\n", &clientdirection, &pari_faceDes,
               &pari_nbrDes);
        idClient = clientdirection;
        // print("%d %d\n",pari_faceDes, pari_nbrDes);
        p_tmp->faceDes = pari_faceDes;
        p_tmp->nbrDes = pari_nbrDes;
        p_tmp->c = &udpClients[idClient];
        sprintf(message, "P %d %d %d \n", idClient, p_tmp->faceDes,
                p_tmp->nbrDes);
        idClient = get_next_idClient(enListe, idClient);
        enListe[clientdirection] = 0;
        broadcast(udpClients, message, enListe, idClient, 1);
        // print("%s\n", message);
        // on interroge le joueur suivant :
        sprintf(requete, "T %d\n", idClient); // dit qui doit parier
        broadcast(udpClients, requete, enListe, idClient, 1);
        enListe[clientdirection] = 1;
        // print("%s\n", requete);
        break;
      case 'D': // on reçoit les valeurs des dés d'un joueur
                // print("nombre de clients : %d\n", nbrClients);
        sscanf(buffer, "D %d %d %d %d %d %d\n", &clientdirection, &D[0], &D[1],
               &D[2], &D[3], &D[4]);
        idClient = clientdirection;
        int i = clientdirection;
        for (int j = 0; j < 5; ++j)
          valDes[5 * i + j] = D[j];

        // print("valDes : %d\n",valDes[i*(idClient+1)]);
        init_table_des(D);
        // print("\n");
        // print("tabDes : %d %d %d %d %d
        // %d\n",tableDes[0],tableDes[1],tableDes[2],tableDes[3],tableDes[4],tableDes[5]);
        tirage_recu++;
        if (tirage_recu == nbrClients)
        {
          tirage_recu = 0;
          if (lock_pacifico == 1)
          { // si un joueur est un pacifico, c'est son tour de jouer
            sprintf(anonce_pacifico, "T %d\n", id_pacifico);
            broadcast(udpClients, anonce_pacifico, enListe, idClient, 0);
            // print("%s\n",anonce_pacifico);
            id_pacifico = 0;
            lock_pacifico = 0;
          }
          if (lock_pacifico == 0 && permier_tour == 0)
          { // sinon on demande au perdant de jouer, sinon au joueur suivant
            if (enListe[idPerdant] == 1)
            {
              sprintf(anonce_tour, "T %d\n", idPerdant); // dit qui doit parier
              broadcast(udpClients, anonce_tour, enListe, idClient, 0);
              // print("%s\n",anonce_tour);
            }
            else
            {
              sprintf(
                  anonce_tour, "T %d\n",
                  get_next_idClient(enListe, idPerdant)); // dit qui doit parier
              broadcast(udpClients, anonce_tour, enListe, idClient, 0);
              // print("%s\n",anonce_tour);
            }
          }
          if (permier_tour == 1)
          { // si premier tour on questionne le joueur tiré au hasard

            sprintf(anonce_tour, "T %d premier tour\n", idPremierJoueur);
            broadcast(udpClients, anonce_tour, enListe, idClient, 0);
            // print("%s\n", anonce_tour);
            permier_tour = 0;
          }
          idClient = idPremierJoueur;
        }
        break;
      }
    }

    // printf("Client: %s", buffer);
  }
  close(sockfd);

  return 0;
}
