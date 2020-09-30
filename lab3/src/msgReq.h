#ifndef __MSGREQ_H__
#define __MSGREQ_H__

#include "common.h"

class msgReq_c
{
public:
  msgReq_c(string method, string URI, string HTTPVersion);
  msgReq_c(string msgString);
  string getMsgString();
  string getURI() { return "." + URI; }

private:
  string method;
  string URI;
  string HTTPVersion;

};

#endif
