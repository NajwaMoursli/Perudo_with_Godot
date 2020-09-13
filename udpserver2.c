// Server side implementation of an UDP client-server model
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 4241
#define MAXLINE 1024

void sendMessageToGodotClient(char *hostname, int portno, char *mess)
{
  /* socket: create the socket */
  int socketGodot;
  struct hostent *serverGodot;
  int serverlen;
  struct sockaddr_in serverGodotAddr;
  int n;
  // char sendBuffer[MAXLINE];

  socketGodot = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketGodot < 0)
  {
    printf("ERROR opening socketGodot\n");
    exit(EXIT_FAILURE);
  }

  /* gethostbyname: get the server's DNS entry */
  serverGodot = gethostbyname(hostname);
  if (serverGodot == NULL)
  {
    fprintf(stderr, "ERROR, no such host as %s\n", hostname);
    exit(EXIT_FAILURE);
  }

  /* build the server's Internet address */
  bzero((char *)&serverGodotAddr, sizeof(serverGodotAddr));
  serverGodotAddr.sin_family = AF_INET;
  bcopy((char *)serverGodot->h_addr, (char *)&serverGodotAddr.sin_addr.s_addr,
        serverGodot->h_length);
  serverGodotAddr.sin_port = htons(portno);

  /* send the message to the server */
  serverlen = sizeof(serverGodotAddr);
  n = sendto(socketGodot, mess, strlen(mess), 0,
             (struct sockaddr *)&serverGodotAddr, serverlen);
  if (n < 0)
  {
    printf("ERROR in sendto\n");
    exit(EXIT_FAILURE);
  }
  close(socketGodot);
}

int main()
{
  int sockfd;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr, cliaddr;

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

  while (1)
  {
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    // printf("n=%d\n",n);
    buffer[n] = '\0';

    printf("Client: %s", buffer);

    sendMessageToGodotClient("192.168.1.16", 4000, "[Server]: received.");
  }
  close(sockfd);
  return 0;
}
