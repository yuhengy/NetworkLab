#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "config.h"
#include "common.h"

class socketClient_c {
public:
  socketClient_c();
  void mySend(string msgString);
  string myReceive();
  ~socketClient_c();

private:
  int sock;
};