#include "msgReq.h"

msgReq_c::msgReq_c(string method, string URI, string HTTPVersion) :
  method(method), URI(URI), HTTPVersion(HTTPVersion) {};

msgReq_c::msgReq_c(string msgString)
{
  vector<string> msgSplitString = split(msgString, "\r\n\r\n");
  string header = msgSplitString[0];

  vector<string> headerSpliteString = split(header, "\r\n");
  string statusLine = headerSpliteString[0];

  vector<string> statusLineSplitString = split(statusLine, " ");
  method  = statusLineSplitString[0];
  URI     = statusLineSplitString[1];
  HTTPVersion = statusLineSplitString[2];
}

string msgReq_c::getMsgString()
{
  return method + " " + URI + " " + HTTPVersion + "\r\n"
       + "\r\n";
}
