#ifndef __TCPSOCK_H__
#define __TCPSOCK_H__

class TCPPacketModule_c;
class TCPApp_c;
#include "iface.h"
#include <stdint.h>
#include <thread>


struct __attribute__((packed)) sock_addr {
  uint32_t IP;
  uint16_t port;
};

struct tcp_sock {
  // connection
  struct sock_addr local;
  struct sock_addr peer;
};


class TCPSock_c {
public:
  TCPSock_c(iface_c* _iface) { iface = _iface; }
  void addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule) { TCPPacketModule = _TCPPacketModule; }
  void addTCPApp(TCPApp_c* _TCPApp) { TCPApp = _TCPApp; }

  void handlePacket(
    char* TCPSockPacket, int TCPSockPacketLen,
    uint32_t sIP, uint16_t sPort,
    uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
  );

  // Application Interface
  struct tcp_sock* alloc_tcp_sock();
  int tcp_sock_bind(struct tcp_sock* tsk, struct sock_addr* sockAddr);
  int tcp_sock_listen(struct tcp_sock *tsk, int backlog);
  struct tcp_sock* tcp_sock_accept(struct tcp_sock *tsk);
  int tcp_sock_connect(struct tcp_sock *tsk, struct sock_addr *skaddr);
  void tcp_sock_close(struct tcp_sock *tsk);


private:
  iface_c* iface;
  TCPPacketModule_c* TCPPacketModule;
  TCPApp_c* TCPApp;

};

#endif
