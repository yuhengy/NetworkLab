#include "socketServerSubhandler.h"

string socketServerSubhandler_c::myReceive(int* len)
{
  char receiveMsg[RESPMSG_LEN];
  memset(receiveMsg, 0, sizeof(receiveMsg));
  *len = recv(cs, receiveMsg, RESPMSG_LEN, 0);
  return string(receiveMsg);
}

void socketServerSubhandler_c::mySend(string msgString)
{
  write(cs, msgString.c_str(), msgString.size());
}
