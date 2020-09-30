#include "socketClient.h"

socketClient_c::socketClient_c()
{
  // create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);  //TODO: Why put this line in `if()` will fail!??
  if (sock == -1) {
      printf("create socket failed\n");
  }
  printf("socket created\n");
     
  // connect to server
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr(SERVERIPADDR);
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVERPORT);
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
      printf("connect failed\n");
  }
  printf("connected\n");
}

void socketClient_c::mySend(string msgString)
{
  if (send(sock, msgString.c_str(), msgString.size(), 0) < 0) {
    printf("send failed\n");
  }
}

string socketClient_c::myReceive()
{
  char receiveMsg[RESPMSG_LEN];
  memset(receiveMsg, 0, sizeof(receiveMsg));
  int len = recv(sock, receiveMsg, RESPMSG_LEN, 0);
  if (len < 0) {
    printf("recv failed\n");
  }
  return string(receiveMsg);
}


socketClient_c::~socketClient_c()
{
  close(sock);
}



