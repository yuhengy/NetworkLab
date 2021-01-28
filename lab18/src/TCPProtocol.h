#ifndef __TCPSOCK_H__
#define __TCPSOCK_H__

class TCPPacketModule_c;
class TCPApp_c;
#include "common.h"
#include "iface.h"
#include "congestionControl.h"
#include <stdint.h>
#include <assert.h>
#include <list>
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

class ringBuffer_t {
public:
  uint32_t size;
  uint32_t head;   // read from head, for application
  uint32_t tail;   // write from tail, i.e. start of recv window
  char* buf;
  bool* filled;

  // set the buf, dest/source, filled
  // but not change head, tail
  void readFromBuffer(char* dest, uint32_t sourceIndex, int len) {
    assert(len < size);
    sourceIndex = sourceIndex % size;
    for (int i = 0; i < len; i++, sourceIndex++, dest++) {
      if (sourceIndex == size) sourceIndex = 0;
      *dest = buf[sourceIndex];
      filled[sourceIndex] = false;
    }
  }
  void writeToBuffer(uint32_t destIndex, char* source, int len) {
    assert(len < size);
    destIndex = destIndex % size;
    for (int i = 0; i < len; i++, source++, destIndex++) {
      if (destIndex == size) destIndex = 0;
      buf[destIndex] = *source;
      filled[destIndex] = true;
    }
  }
};

struct tcp_sock;
class TCPPacketInfo_c {
public:
  char*    logPacket;
  int      logPacketLen;
  uint32_t logSeq;
  uint8_t  logFlags;

  int resendTime = 0;

  void sendAndLogPacket(
    tcp_sock* TCPSock, TCPPacketModule_c* TCPPacketModule,
    char* packet, int packetLen, uint8_t flags
  );
  void resend(tcp_sock* TCPSock, TCPPacketModule_c* TCPPacketModule);
  void freeLogPacket() {
    free(logPacket - (ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN));
  }

};

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

  ringBuffer_t ringBuffer;
  // the most recent ack from other side
  uint32_t snd_una;
  // the seq to send next
  uint32_t snd_nxt;
  // the ack to send next, equal to the seq expected to receive next
  uint32_t rcv_nxt;
  uint16_t rcv_wnd;

  uint8_t  timeWait;

  bool      timingOpen;
  uint32_t  ACKTimeOut;  // in ms
  uint32_t  ACKPendingTime;  // in ms
  std::list<TCPPacketInfo_c> sendBuffer;

  congestionControl_c congestionControl;
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
  void handlePacket_handleACK(tcp_sock* TCPSock, uint32_t ack);

  // Application Interface
  struct tcp_sock* alloc_tcp_sock();
  int tcp_sock_bind(struct tcp_sock* TCPSock, struct sock_addr* localSockAddr);
  int tcp_sock_listen(struct tcp_sock* TCPSock, int backlog);
  struct tcp_sock* tcp_sock_accept(struct tcp_sock *TCPSock);
  int tcp_sock_connect(struct tcp_sock* TCPSock, struct sock_addr *peerSockAddr);
  void tcp_sock_close(struct tcp_sock* TCPSock);

  int tcp_sock_read(struct tcp_sock* tsk, char* buf, int len);
  int tcp_sock_write(struct tcp_sock* tsk, char* buf, int len);

  void debug_printTCPSock(tcp_sock* TCPSock);


private:
  iface_c* iface;
  TCPPacketModule_c* TCPPacketModule;
  TCPApp_c* TCPApp;

  // sub threads
  void timeWaitThread();
  void ACKTimeOutThread();
  void ACKTimeOutThread_handleTImeOut(tcp_sock* TCPSock);
  std::thread timeWait, ACKTimeOut;

  // All current TCP link
  std::map<sock_addr, tcp_sock*> listenTable;
  std::map<std::pair<sock_addr, sock_addr>, tcp_sock*> establishedTable;
  std::mutex TCPSockTable_mutex;

  // client port
  uint16_t nextClientPort = 49152;

  // handle packet
  // NOTE: TCPSockTable_mutex is already hold when entering following functions
  void handleSYN1(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack);
  void handleSYN2(tcp_sock* TCPSock, uint32_t ack);
  void handleSYN3(tcp_sock* TCPSock, uint32_t seq, uint16_t rwnd);
  void handleSYN4(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack, uint16_t rwnd);
  void handleFIN1(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN2(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN3(tcp_sock* TCPSock, uint32_t seq, uint32_t ack);
  void handleFIN4(tcp_sock* TCPSock);
  void handleSaveData(
    tcp_sock* TCPSock, uint32_t seq, uint32_t ack,
    uint16_t rwnd, char* TCPProtocolPacket, int TCPProtocolPacketLen
  );
};

#endif
