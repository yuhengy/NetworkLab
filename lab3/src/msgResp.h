#ifndef __MSGRESP_H__
#define __MSGRESP_H__

#include "common.h"

class msgResp_c
{
public:
  msgResp_c(string method, string URI, string HTTPVersion);
  msgResp_c(string msgString);
  void readBodyFromFile(string fileName);
  void writeBodyToFIle(string fileName);
  string getMsgString();
  bool getSuccess() { return statusCode.compare("404")!=0; }

private:
  string HTTPVersion;
  string statusCode;
  string reasonPhrase;
  string contentLength;
  string body;

};

#endif
