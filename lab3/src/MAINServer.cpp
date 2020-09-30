#include "common.h"
#include "msgReq.h"
#include "msgResp.h"
#include "socketServer.h"

int main(int argc, char *argv[])
{
  // STEP1 create socket
  socketServer_c socketServer = socketServer_c();

  while(true) {
    // STEP2 receive request
    int len;
    string msg = socketServer.myReceive(&len);
    if (len < 0) {
      printf("recv failed\n");
      break;
    } 
    else if (len == 0) {
      printf("client disconnected\n");
      break;
    }
    printf("=======All Reveived Msg=======\n");
    printf("%s\n", msg.c_str());
    msgReq_c msgReq = msgReq_c(msg);
    printf("=======But, I only use=======\n%s\n========================\n", msgReq.getMsgString().c_str());

    // STEP3 read text
    msgResp_c* msgResp_p;
    ifstream file(msgReq.getURI());
    if (file.good()){
      msgResp_p = new msgResp_c("HTTP/1.1", "200", "OK");
      msgResp_p->readBodyFromFile(msgReq.getURI());
    }
    else {
      msgResp_p = new msgResp_c("HTTP/1.1", "404", "File not found");
    }

    // STEP4 send response
    socketServer.mySend(msgResp_p->getMsgString());
    printf("=======sendMsg=======\n%s\n=====================\n", msgResp_p->getMsgString().c_str());
  }

  return 0;
}
