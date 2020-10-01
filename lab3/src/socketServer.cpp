#include "socketServer.h"

socketServer_c::socketServer_c()
{
  // create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("create socket failed\n");
  }
  printf("socket created\n");
     
  // connect to server
  struct sockaddr_in server, client;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(SERVERPORT);

  // bind
  if (bind(sock,(struct sockaddr *)&server, sizeof(server)) < 0) {
      perror("bind failed\n");
  }
  printf("bind done\n");

  // listen
  listen(sock, 3);
  printf("waiting for incoming connections...\n");
}

long socketServer_c::waitNextClient()
{
  // connect to server
  struct sockaddr_in client;
  long cs;

  // accept connection from an incoming client
  int c = sizeof(struct sockaddr_in);
  if ((cs = accept(sock, (struct sockaddr *)&client, (socklen_t *)&c)) < 0) {
      perror("accept failed\n");
  }
  printf("connection accepted\n");
  return cs;
}
