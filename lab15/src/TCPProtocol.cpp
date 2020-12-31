#include "TCPProtocol.h"

#include "common.h"
#include "TCPPacketModule.h"
#include <unistd.h>

# define TCP_FIN  0x01
# define TCP_SYN  0x02
# define TCP_RST  0x04
# define TCP_PSH  0x08
# define TCP_ACK  0x10
# define TCP_URG  0x20

# define RWND (1024 * 16)

void TCPProtocol_c::handlePacket(
  char* TCPProtocolPacket, int TCPProtocolPacketLen,
  uint32_t sIP, uint16_t sPort,
  uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
)
{

#if 1
  printf("******************************************************\n");
  printf("*************TCPProtocol_c::handlePacket start************\n");
  printf("******************************************************\n");
  printf("****************************************************\n");
  printf("*************TCPProtocol_c::handlePacket end************\n");
  printf("****************************************************\n");
#endif

}


struct tcp_sock* TCPProtocol_c::alloc_tcp_sock()
{
  tcp_sock* TCPSock = new tcp_sock;
  TCPSock->TCPState = TCP_CLOSED;
  return TCPSock;
}


int TCPProtocol_c::tcp_sock_bind(struct tcp_sock* TCPSock, struct sock_addr* sockAddr)
{
  TCPSock->localAddr.port = sockAddr->port;
  return 0;
}


int TCPProtocol_c::tcp_sock_listen(struct tcp_sock *TCPSock, int backlog)
{
  TCPSock->TCPState = TCP_LISTEN;
  TCPSock->localAddr.IP = iface->getIP();

  TCPSockTable_mutex.lock();
  auto iter = listenTable.find(TCPSock->localAddr);
  if (iter != listenTable.end()) {
    TCPSockTable_mutex.unlock();
    return -1;
  }
  else {
    listenTable[TCPSock->localAddr] = TCPSock;
    TCPSockTable_mutex.unlock();
    return 0;
  }
}


struct tcp_sock* TCPProtocol_c::tcp_sock_accept(struct tcp_sock *TCPSock)
{
  // no racing, because only one producer and only this consumer
  while (TCPSock->acceptList.size() == 0) ;

  TCPSockTable_mutex.lock();
  tcp_sock* TCPSockChild = TCPSock->acceptList.begin()->second;
  TCPSockChild->peerAddr = TCPSock->acceptList.begin()->first;
  TCPSock->acceptList.erase(TCPSock->acceptList.begin());
  establishedTable[std::make_pair(TCPSockChild->localAddr, TCPSockChild->peerAddr)] = TCPSockChild;
  TCPSockTable_mutex.unlock();

  return TCPSockChild;
}


int TCPProtocol_c::tcp_sock_connect(struct tcp_sock *TCPSock, struct sock_addr *sockAddr)
{

  // STEP1 initialize the four key tuple (sip, sport, dip, dport)
  TCPSock->localAddr.IP = iface->getIP();
  TCPSock->localAddr.port = nextClientPort++;
  TCPSock->peerAddr = *sockAddr;

  // STEP2 hash the tcp sock into table
  TCPSockTable_mutex.lock();
  listenTable[TCPSock->localAddr] = TCPSock;

  // STEP3 send SYN packet, switch to TCP_SYN_SENT state, wait for the incoming
  //       SYN packet by sleep on wait_connect
  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
    false, ++(TCPSock->snd_nxt), 0, TCP_SYN, RWND
  );
  TCPSock->TCPState = TCP_SYN_SENT;
  TCPSockTable_mutex.unlock();

  // STEP4 if the SYN packet of the peer arrives, this function is notified, which
  //       means the connection is established
  while (TCPSock->TCPState != TCP_ESTABLISHED) ;
  TCPSockTable_mutex.lock();
  listenTable.erase(TCPSock->localAddr);
  establishedTable[std::make_pair(TCPSock->localAddr, TCPSock->peerAddr)] = TCPSock;
  TCPSockTable_mutex.unlock();
  return 0;
}


void TCPProtocol_c::tcp_sock_close(struct tcp_sock *TCPSock)
{
  TCPSockTable_mutex.lock();
  // corner cases that we have not implemented
  // Correct behaviour is not achieved
  if (TCPSock->TCPState == TCP_SYN_RECV || TCPSock->TCPState == TCP_SYN_SENT) {
    printf("Error: close a TCP during SYN period.\n");
  }
  else if (TCPSock->SynRcvdList.size() != 0) {
    printf("Error: close a TCP during children's SYN period.\n");
  }
  else if (TCPSock->acceptList.size() != 0) {
    printf("Error: close a TCP, but children are in established stage.\n");
  }
  else if (TCPSock->TCPState == TCP_CLOSE_WAIT || TCPSock->TCPState == TCP_LAST_ACK   ||
           TCPSock->TCPState == TCP_FIN_WAIT_1 || TCPSock->TCPState == TCP_FIN_WAIT_2 ||
           TCPSock->TCPState == TCP_CLOSING    || TCPSock->TCPState == TCP_TIME_WAIT    ) {
    printf("Error: close a TCP, but it already in closing period.\n");
  }

  // one example is the parent TCP of server listening
  else if (TCPSock->TCPState == TCP_LISTEN) {
    listenTable.erase(TCPSock->localAddr);
  }

  // usual case, closed by other side
  else if (TCPSock->TCPState == TCP_CLOSED) {
    establishedTable.erase(std::make_pair(TCPSock->localAddr, TCPSock->peerAddr));
  }

  // usual case, closed by this side
  else if (TCPSock->TCPState == TCP_ESTABLISHED) {
    char *emptyPacket = (char*)malloc(
      ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
    );
    emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
    TCPPacketModule->sendPacket(
      emptyPacket, 0,
      TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
      false, ++(TCPSock->snd_nxt), TCPSock->rcv_nxt, TCP_SYN | TCP_ACK, RWND
    );
    TCPSock->TCPState = TCP_FIN_WAIT_1;
    TCPSockTable_mutex.unlock();

    while (TCPSock->TCPState != TCP_CLOSED) ;
    TCPSockTable_mutex.lock();
    establishedTable.erase(std::make_pair(TCPSock->localAddr, TCPSock->peerAddr));
  }

  TCPSockTable_mutex.unlock();
}






