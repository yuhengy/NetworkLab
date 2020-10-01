#include "common.h"
#include "msgReq.h"
#include "msgResp.h"
#include "socketServer.h"
#include "socketServerSubhandler.h"

void *handleConnection(void *cs)
{
  socketServerSubhandler_c socketServerSubhandler = socketServerSubhandler_c((long)cs);

  // STEP3 receive request
  int len;
  string msg = socketServerSubhandler.myReceive(&len);
  if (len < 0) {
    printf("recv failed\n");
    return NULL;
  } 
  else if (len == 0) {
    printf("client disconnected\n");
    return NULL;
  }
  printf("=======All Reveived Msg=======\n");
  printf("%s\n", msg.c_str());
  msgReq_c msgReq = msgReq_c(msg);
  printf("=======But, I only use=======\n%s\n========================\n", msgReq.getMsgString().c_str());

  // STEP4 read text
  msgResp_c* msgResp_p;
  ifstream file(msgReq.getURI());
  if (file.good()){
    msgResp_p = new msgResp_c("HTTP/1.1", "200", "OK");
     msgResp_p->readBodyFromFile(msgReq.getURI());
  }
  else {
    msgResp_p = new msgResp_c("HTTP/1.1", "404", "File not found");
  }

  // THIS is used to check requests are served in parallel
  sleep(10);

  // STEP5 send response
  socketServerSubhandler.mySend(msgResp_p->getMsgString());
  printf("=======sendMsg=======\n%s\n=====================\n", msgResp_p->getMsgString().c_str());
  return NULL;

}

int main(int argc, char *argv[])
{
  // STEP0 init threads
  pthread_t threads[NUM_THREADS];
  int threadIndex = 0;

  // STEP1 create socket
  socketServer_c socketServer = socketServer_c();


  // STEP2 connection request
  while (long cs = socketServer.waitNextClient()) {
    int ret = pthread_create(&threads[threadIndex], NULL, handleConnection, (void *)cs);
    if (ret != 0)
    {
      printf("pthread_create error: error_code=%d", ret);
    }
    threadIndex++;
  }
  pthread_exit(NULL);
  return 0;
}
