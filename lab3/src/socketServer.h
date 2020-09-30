#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "config.h"
#include "common.h"

class socketServer_c {
public:
  socketServer_c();
  string myReceive(int* len);
  void mySend(string msgString);

private:
  int cs;
};