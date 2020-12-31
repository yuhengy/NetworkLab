#ifndef __TCPSOCK_H__
#define __TCPSOCK_H__

class TCPPacketModule_c;
class TCPApp_c;
#include "iface.h"
#include <stdint.h>
#include <map>
#include <thread>
#include <mutex>



struct sock_addr {
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

  std::map<sock_addr, tcp_sock*> synRcvdList;
  std::map<sock_addr, tcp_sock*> acceptList;

  // the most recent ack from other side
  uint32_t snd_una;

  // the seq to send next
  uint32_t snd_nxt;
  // the ack to send next, equal to the seq expected to receive next
  uint32_t rcv_nxt;

  uint8_t  timeWait;



};


class TCPProtocol_c {
public:
  TCPProtocol_c(iface_c* _iface) { iface = _iface; }
  void addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule) { TCPPacketModule = _TCPPacketModule; }
  void addTCPApp(TCPApp_c* _TCPApp) { TCPApp = _TCPApp; }

  void startSubthread();

  void handlePacket(
    char* TCPProtocolPacket, int TCPProtocolPacketLen,
    uint32_t sIP, uint16_t sPort, uint16_t dPort,
    uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
  );

  // Application Interface
  struct tcp_sock* alloc_tcp_sock();
  int tcp_sock_bind(struct tcp_sock* TCPSock, struct sock_addr* localSockAddr);
  int tcp_sock_listen(struct tcp_sock *TCPSock, int backlog);
  struct tcp_sock* tcp_sock_accept(struct tcp_sock *TCPSock);
  int tcp_sock_connect(struct tcp_sock *TCPSock, struct sock_addr *peerSockAddr);
  void tcp_sock_close(struct tcp_sock *TCPSock);



private:
  iface_c* iface;
  TCPPacketModule_c* TCPPacketModule;
  TCPApp_c* TCPApp;

  // sub threads
  void timeWaitThread();
  std::thread timeWait;

  // All current TCP link
  std::map<sock_addr, tcp_sock*> listenTable;
  std::map<std::pair<sock_addr, sock_addr>, struct tcp_sock*> establishedTable;
  std::mutex TCPSockTable_mutex;

  // client port
  uint16_t nextClientPort = 49152;

  // handle packet
  // NOTE: TCPSockTable_mutex is already hold when entering following functions
  void handleSYN1(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack);
  void handleSYN2(tcp_sock* TCPSock, uint32_t ack);
  void handleSYN3(tcp_sock* TCPSock, uint32_t seq);
  void handleSYN4(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack);
  void handleFIN1(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN2(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN3(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN4(tcp_sock* TCPSock);

};

#endif
