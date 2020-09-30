#include "common.h"
#include "msgReq.h"
#include "msgResp.h"
#include "socketClient.h"

int main(int argc, char *argv[])
{
  if (argc <= 2) {
    printf("Error: 2 arguments needed!\n");
    return 0;
  }
  // STEP1 create socket
  socketClient_c socketClient = socketClient_c();

  // STEP2 send request
  msgReq_c msgReq = msgReq_c("GET", argv[1], "HTTP/1.1");
  socketClient.mySend(msgReq.getMsgString());
  printf("=======sendMsg=======\n%s\n=====================\n", msgReq.getMsgString().c_str());

  // STEP3 receive response
  printf("=======All Reveived Msg=======\n");
  string msg = socketClient.myReceive();
  printf("%s\n", msg.c_str());
  msgResp_c msgResp = msgResp_c(msg);
  printf("-------But, I only use-------\n%s\n========================\n", msgResp.getMsgString().c_str());

  // STEP4 save text
  msgResp.writeBodyToFIle(argv[2]);

  return 0;
}
