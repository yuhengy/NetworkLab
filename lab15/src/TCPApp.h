#ifndef __TCPAPP_H__
#define __TCPAPP_H__

class TCPSock_c;
#include <stdint.h>
#include <thread>



class TCPApp_c {
public:
  void addTCPSock(TCPSock_c* _TCPSock) { TCPSock = _TCPSock; }

  void startServerthread(void *arg) {
    serverThread = std::thread(&TCPApp_c::TCPServer, this, arg);
  }
  void startClientthread(void *arg) {
    clientThread = std::thread(&TCPApp_c::TCPClient, this, arg);
  }


private:
  TCPSock_c* TCPSock;

  // sub threads
  void TCPServer(void *arg);
  void TCPClient(void *arg);
  std::thread serverThread, clientThread;

};

#endif
