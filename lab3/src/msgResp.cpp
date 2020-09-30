#include "msgResp.h"

msgResp_c::msgResp_c(string HTTPVersion, string statusCode, string reasonPhrase) :
  HTTPVersion(HTTPVersion), statusCode(statusCode), reasonPhrase(reasonPhrase) {};

msgResp_c::msgResp_c(string msgString)
{
  vector<string> msgSplitString = split(msgString, "\r\n\r\n");
  string header = msgSplitString[0];
  body = msgSplitString[1];

  vector<string> headerSpliteString = split(header, "\r\n");
  string statusLine = headerSpliteString[0];

  vector<string> statusLineSplitString = split(statusLine, " ");
  HTTPVersion  = statusLineSplitString[0];
  statusCode   = statusLineSplitString[1];
  reasonPhrase = statusLineSplitString[2];
}

void msgResp_c::readBodyFromFile(string fileName)
{
  ifstream in(fileName);
  if (!in.good()) {
    printf("Error: Cannot open file.\n");
  }
  getline(in, body, '\0');
  //ifstream file(fileName);
  //ostringstream temp;
  //temp << file.rdbuf();
  //body = temp.str();
  contentLength = "Content-Length: " + to_string(body.size());
}

void msgResp_c::writeBodyToFIle(string fileName)
{
  ofstream out(fileName);
  if (!out.good()) {
    printf("Error: Cannot open file.\n");
  }
  out << body;
  out.close();
}

string msgResp_c::getMsgString()
{
  return HTTPVersion + " " + statusCode + " " + reasonPhrase + "\r\n"
       + contentLength + "\r\n"
       + "\r\n"
       + body;
}
