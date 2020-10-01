#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "config.h"
#include "common.h"

class socketServerSubhandler_c {
public:
  socketServerSubhandler_c(long cs) : cs(cs) {};
  string myReceive(int* len);
  void mySend(string msgString);

private:
  long cs;
};