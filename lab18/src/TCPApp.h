#ifndef __TCPAPP_H__
#define __TCPAPP_H__

class TCPProtocol_c;
#include <stdint.h>
#include <thread>



class TCPApp_c {
public:
  void addTCPProtocol(TCPProtocol_c* _TCPProtocol) { TCPProtocol = _TCPProtocol; }

  void startServerthread(void *arg) {
    serverThread = std::thread(&TCPApp_c::TCPServer, this, arg);
  }
  void startClientthread(void *arg) {
    clientThread = std::thread(&TCPApp_c::TCPClient, this, arg);
  }


private:
  TCPProtocol_c* TCPProtocol;

  // sub threads
  void TCPServer(void *arg);
  void TCPClient(void *arg);
  std::thread serverThread, clientThread;

};

#endif
