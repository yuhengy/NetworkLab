#ifndef __TCPSOCK_H__
#define __TCPSOCK_H__

class TCPPacketModule_c;
class TCPApp_c;
#include "iface.h"
#include <stdint.h>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>



struct __attribute__((packed)) sock_addr {
  uint32_t IP;
  uint16_t port;
};
static bool operator< (sock_addr lhs, sock_addr rhs) {
  return lhs.IP < rhs.IP || (lhs.IP == rhs.IP && lhs.port < rhs.port);
}


// tcp states
enum TCPState_c {
  TCP_CLOSED, TCP_LISTEN, TCP_SYN_RECV, TCP_SYN_SENT,
  TCP_ESTABLISHED, TCP_CLOSE_WAIT, TCP_LAST_ACK, TCP_FIN_WAIT_1,
  TCP_FIN_WAIT_2, TCP_CLOSING, TCP_TIME_WAIT };

struct tcp_sock {
  enum TCPState_c TCPState;

  // connection
  sock_addr localAddr;
  sock_addr peerAddr;

  std::map<sock_addr, tcp_sock*> SynRcvdList;
  std::map<sock_addr, tcp_sock*> acceptList;

  // the highest byte that is ACKed by peer
  uint32_t snd_una;
  // the highest byte sent
  uint32_t snd_nxt;
  // the highest byte ACKed by itself (i.e. the byte expected to receive next)
  uint32_t rcv_nxt;

};


class TCPProtocol_c {
public:
  TCPProtocol_c(iface_c* _iface) { iface = _iface; }
  void addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule) { TCPPacketModule = _TCPPacketModule; }
  void addTCPApp(TCPApp_c* _TCPApp) { TCPApp = _TCPApp; }

  void handlePacket(
    char* TCPProtocolPacket, int TCPProtocolPacketLen,
    uint32_t sIP, uint16_t sPort,
    uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
  );

  // Application Interface
  struct tcp_sock* alloc_tcp_sock();
  int tcp_sock_bind(struct tcp_sock* TCPSock, struct sock_addr* sockAddr);
  int tcp_sock_listen(struct tcp_sock *TCPSock, int backlog);
  struct tcp_sock* tcp_sock_accept(struct tcp_sock *TCPSock);
  int tcp_sock_connect(struct tcp_sock *TCPSock, struct sock_addr *sockAddr);
  void tcp_sock_close(struct tcp_sock *TCPSock);


private:
  iface_c* iface;
  TCPPacketModule_c* TCPPacketModule;
  TCPApp_c* TCPApp;

  // All current TCP link
  std::map<sock_addr, tcp_sock*> listenTable;
  std::map<std::pair<sock_addr, sock_addr>, struct tcp_sock*> establishedTable;
  std::mutex TCPSockTable_mutex;

  // client port
  uint16_t nextClientPort = 49152;

  // handle packet
  //void handleSYN1();

};

#endif
