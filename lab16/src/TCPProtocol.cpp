#include "TCPProtocol.h"

#include "common.h"
#include "TCPPacketModule.h"
#include <unistd.h>
#include <string.h>

# define TCP_FIN  0x01
# define TCP_SYN  0x02
# define TCP_RST  0x04
# define TCP_PSH  0x08
# define TCP_ACK  0x10
# define TCP_URG  0x20

# define RWND RINGBUFFER_SIZE
# define RINGBUFFER_SIZE (1024)
# define MAX_DATA_SIZE (1514 - (ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN))

#define TCP_MSL 100

void TCPProtocol_c::startSubthread()
{
  timeWait = std::thread(&TCPProtocol_c::timeWaitThread, this);
}

void TCPProtocol_c::timeWaitThread()
{
  while (true) {
    sleep(1);

    TCPSockTable_mutex.lock();

    for (auto iter=establishedTable.begin(); iter!=establishedTable.end(); iter++)
      if (iter->second->TCPState == TCP_TIME_WAIT)
        iter->second->timeWait++;

    for (auto iter=establishedTable.begin(); iter!=establishedTable.end(); iter++)
      if (iter->second->TCPState == TCP_TIME_WAIT)
        if (iter->second->timeWait++ > 2*TCP_MSL)
          iter->second->TCPState = TCP_CLOSED;

    TCPSockTable_mutex.unlock();
  }
}

//--------------------------------------------------------------------

void TCPProtocol_c::handlePacket(
  char* TCPProtocolPacket, int TCPProtocolPacketLen,
  uint32_t sIP, uint16_t sPort, uint16_t dPort,
  uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
)
{

  sock_addr localSockAddr = {iface->getIP(), dPort};
  sock_addr peerSockAddr = {sIP, sPort};

  TCPSockTable_mutex.lock();
  auto iter1 = establishedTable.find(std::make_pair(localSockAddr, peerSockAddr));
  auto iter2 = listenTable.find(localSockAddr);
  tcp_sock* TCPSock;

  // during establish stage
  if (iter1 != establishedTable.end()) {
    tcp_sock* TCPSock = iter1->second;
    
#if 0
    printf("******************************************************\n");
    printf("***********TCPProtocol_c::handlePacket start**********\n");
    printf("******************************************************\n");
    debug_printTCPSock(TCPSock);
    printf("****************************************************\n");
    printf("***********TCPProtocol_c::handlePacket end**********\n");
    printf("****************************************************\n");
#endif

    if (TCPSock->TCPState == TCP_ESTABLISHED && (flags & TCP_FIN))
      handleFIN1(TCPSock, seq, ack);
    else if (TCPSock->TCPState == TCP_ESTABLISHED && !(flags & TCP_FIN))
      handleSaveData(TCPSock, seq, ack, rwnd, TCPProtocolPacket, TCPProtocolPacketLen);
    else if (TCPSock->TCPState == TCP_FIN_WAIT_1) {
      handleFIN2(TCPSock, seq, ack);
      if ((TCPSock->TCPState == TCP_FIN_WAIT_1 || TCPSock->TCPState == TCP_FIN_WAIT_2) && (flags & TCP_FIN))
        handleFIN3(TCPSock, seq, ack);
    }
    else if ((TCPSock->TCPState == TCP_FIN_WAIT_1 || TCPSock->TCPState == TCP_FIN_WAIT_2) && (flags & TCP_FIN))
      handleFIN3(TCPSock, seq, ack);
    else if (TCPSock->TCPState == TCP_LAST_ACK)
      handleFIN4(TCPSock);
  }

  // during listen stage
  else if (iter2 != listenTable.end()) {
    tcp_sock* TCPSock = iter2->second;

#if 0
    printf("******************************************************\n");
    printf("***********TCPProtocol_c::handlePacket start**********\n");
    printf("******************************************************\n");
    debug_printTCPSock(TCPSock);
    printf("****************************************************\n");
    printf("***********TCPProtocol_c::handlePacket end**********\n");
    printf("****************************************************\n");
#endif

    if (TCPSock->TCPState == TCP_LISTEN)
      handleSYN1(TCPSock, peerSockAddr, seq, ack);
    else if (TCPSock->TCPState == TCP_SYN_SENT && (flags & TCP_ACK)) {
      handleSYN2(TCPSock, ack);
      if (TCPSock->TCPState == TCP_SYN_SENT && (flags & TCP_SYN))
        handleSYN3(TCPSock, seq, rwnd);
    }
    else if (TCPSock->TCPState == TCP_SYN_SENT && (flags & TCP_SYN))
      handleSYN3(TCPSock, seq, rwnd);
    else if (TCPSock->TCPState == TCP_SYN_RECV)
      handleSYN4(TCPSock, peerSockAddr, seq, ack, rwnd);
  }

  TCPSockTable_mutex.unlock();
}

//--------------------------------------------------------------------

struct tcp_sock* TCPProtocol_c::alloc_tcp_sock()
{
  tcp_sock* TCPSock = new tcp_sock;
  TCPSock->TCPState = TCP_CLOSED;
  TCPSock->ringBuffer.size = RINGBUFFER_SIZE;
  TCPSock->ringBuffer.head = 0;
  TCPSock->ringBuffer.tail = 0;
  TCPSock->ringBuffer.buf = new char[TCPSock->ringBuffer.size];
  TCPSock->ringBuffer.filled = new bool[TCPSock->ringBuffer.size];
  memset(TCPSock->ringBuffer.filled, 0, TCPSock->ringBuffer.size);
  return TCPSock;
}


int TCPProtocol_c::tcp_sock_bind(struct tcp_sock* TCPSock, struct sock_addr* localSockAddr)
{
  TCPSock->localAddr.port = localSockAddr->port;
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
  TCPSock->acceptList.erase(TCPSock->acceptList.begin());
  TCPSockTable_mutex.unlock();

  return TCPSockChild;
}


int TCPProtocol_c::tcp_sock_connect(struct tcp_sock *TCPSock, struct sock_addr *peerSockAddr)
{

  // STEP1 initialize the four key tuple (sip, sport, dip, dport)
  TCPSock->localAddr.IP = iface->getIP();
  TCPSock->localAddr.port = nextClientPort++;
  TCPSock->peerAddr = *peerSockAddr;

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
    false, (TCPSock->snd_nxt)++, 0, TCP_SYN, RWND
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
  else if (TCPSock->synRcvdList.size() != 0) {
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
      false, (TCPSock->snd_nxt)++, TCPSock->rcv_nxt, TCP_FIN, RWND
    );
    TCPSock->TCPState = TCP_FIN_WAIT_1;
    TCPSockTable_mutex.unlock();

    while (TCPSock->TCPState != TCP_CLOSED) ;
    TCPSockTable_mutex.lock();
    establishedTable.erase(std::make_pair(TCPSock->localAddr, TCPSock->peerAddr));
  }

  TCPSockTable_mutex.unlock();
}

//--------------------------------------------------------------------

int TCPProtocol_c::tcp_sock_read(struct tcp_sock* TCPSock, char* buff, int len)
{
  while (TCPSock->ringBuffer.tail - TCPSock->ringBuffer.head == 0) ;

  TCPSockTable_mutex.lock();

#if 0
  printf("******************************************************\n");
  printf("***********TCPProtocol_c::tcp_sock_read start*********\n");
  printf("******************************************************\n");
  debug_printTCPSock(TCPSock);
  printf("****************************************************\n");
  printf("***********TCPProtocol_c::tcp_sock_read end*********\n");
  printf("****************************************************\n");
#endif

  int readLen = TCPSock->ringBuffer.tail - TCPSock->ringBuffer.head;
  TCPSock->ringBuffer.readFromBuffer(buff, TCPSock->ringBuffer.head, readLen);
  TCPSock->ringBuffer.head += readLen;

  TCPSockTable_mutex.unlock();

  return readLen;
}


int TCPProtocol_c::tcp_sock_write(struct tcp_sock* TCPSock, char* buff, int len)
{
  uint32_t MAXSend = TCPSock->snd_nxt + len;
  uint32_t sendSize;

#if 0
    printf("******************************************************\n");
    printf("**********TCPProtocol_c::tcp_sock_write start*********\n");
    printf("******************************************************\n");
    debug_printTCPSock(TCPSock);
    printf("****************************************************\n");
    printf("**********TCPProtocol_c::tcp_sock_write end*********\n");
    printf("****************************************************\n");
#endif

  while (true) {
    // decide the size
    TCPSockTable_mutex.lock();
    if (TCPSock->snd_nxt == MAXSend) {
      TCPSockTable_mutex.unlock(); break;
    }
    else if (MAX_DATA_SIZE < (MAXSend - TCPSock->snd_nxt) &&
             MAX_DATA_SIZE <= TCPSock->rcv_wnd - (TCPSock->snd_nxt - TCPSock->snd_una)) { // check -+ 1
      sendSize = MAX_DATA_SIZE;
    }
    else if ((MAXSend - TCPSock->snd_nxt) <= MAX_DATA_SIZE &&
             (MAXSend - TCPSock->snd_nxt) <= TCPSock->rcv_wnd - (TCPSock->snd_nxt - TCPSock->snd_una)) {
      sendSize = MAXSend - TCPSock->snd_nxt;
    }
    else {
      TCPSockTable_mutex.unlock();
      continue;
    }

    // send the packet
    char *packet = (char*)malloc(
      sendSize + ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
    );
    packet += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
    memcpy(packet, buff, sendSize);
    TCPPacketModule->sendPacket(
      packet, sendSize,
      TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
      false, TCPSock->snd_nxt, TCPSock->rcv_nxt, TCP_ACK, RWND - (TCPSock->ringBuffer.tail - TCPSock->ringBuffer.head)
    );

    // update log
    TCPSock->snd_nxt += sendSize;

    TCPSockTable_mutex.unlock();
  }

  return len;
}

//--------------------------------------------------------------------

void TCPProtocol_c::handleSYN1(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack)
{
  TCPSock->TCPState  = TCP_SYN_RECV;  //TODO change this
  tcp_sock* TCPSock_child = alloc_tcp_sock();
  TCPSock_child->TCPState  = TCP_SYN_RECV;
  TCPSock_child->localAddr = TCPSock->localAddr;
  TCPSock_child->peerAddr  = peerSockAddr;
  TCPSock_child->rcv_nxt  = seq + 1;
  TCPSock->synRcvdList[TCPSock_child->peerAddr] = TCPSock_child;

  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    TCPSock_child->localAddr.IP, TCPSock_child->localAddr.port, TCPSock_child->peerAddr.IP, TCPSock_child->peerAddr.port,
    false, (TCPSock_child->snd_nxt)++, TCPSock_child->rcv_nxt, TCP_SYN | TCP_ACK, RWND
  );
}


void TCPProtocol_c::handleSYN2(tcp_sock* TCPSock, uint32_t ack)
{
  TCPSock->snd_una = ack;
}


void TCPProtocol_c::handleSYN3(tcp_sock* TCPSock, uint32_t seq, uint16_t rwnd)
{
  TCPSock->TCPState = TCP_ESTABLISHED;
  TCPSock->ringBuffer.head = seq + 1;
  TCPSock->ringBuffer.tail = seq + 1;
  TCPSock->rcv_nxt  = seq + 1;
  TCPSock->rcv_wnd  = rwnd;

  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
    false, TCPSock->snd_nxt, TCPSock->rcv_nxt, TCP_ACK, RWND
  );
}


void TCPProtocol_c::handleSYN4(tcp_sock* TCPSock, sock_addr peerSockAddr, uint32_t seq, uint32_t ack, uint16_t rwnd)
{
  TCPSock->TCPState  = TCP_LISTEN;  //TODO change this
  tcp_sock* TCPSock_child = TCPSock->synRcvdList.find(peerSockAddr)->second;
  TCPSock->synRcvdList.erase(peerSockAddr);
  TCPSock->acceptList[peerSockAddr] = TCPSock_child;
  TCPSock_child->peerAddr = peerSockAddr;
  establishedTable[std::make_pair(TCPSock_child->localAddr, TCPSock_child->peerAddr)] = TCPSock_child;

  TCPSock_child->TCPState = TCP_ESTABLISHED;
  TCPSock_child->snd_una   = ack;
  TCPSock_child->ringBuffer.head = seq;
  TCPSock_child->ringBuffer.tail = seq;
  TCPSock_child->rcv_nxt   = seq;
  TCPSock_child->rcv_wnd   = rwnd;
}


void TCPProtocol_c::handleFIN1(tcp_sock* TCPSock, uint32_t seq, uint32_t ack)
{
  TCPSock->TCPState = TCP_LAST_ACK;
  TCPSock->snd_una  = ack;
  TCPSock->rcv_nxt  = seq + 1;

  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
    false, (TCPSock->snd_nxt)++, TCPSock->rcv_nxt, TCP_FIN | TCP_ACK, RWND
  );
}


void TCPProtocol_c::handleFIN2(tcp_sock* TCPSock, uint32_t seq, uint32_t ack)
{
  TCPSock->TCPState = TCP_FIN_WAIT_2;
  TCPSock->snd_una  = ack;
  //assert(TCPSock->rcv_nxt == seq);
}


void TCPProtocol_c::handleFIN3(tcp_sock* TCPSock, uint32_t seq, uint32_t ack)
{
  TCPSock->TCPState = TCP_TIME_WAIT;
  TCPSock->snd_una  = ack;
  TCPSock->rcv_nxt  = seq + 1;

  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    TCPSock->localAddr.IP, TCPSock->localAddr.port, TCPSock->peerAddr.IP, TCPSock->peerAddr.port,
    false, (TCPSock->snd_nxt)++, TCPSock->rcv_nxt, TCP_ACK, RWND
  );

  TCPSock->TCPState = TCP_TIME_WAIT;
  TCPSock->timeWait = 0;
}


void TCPProtocol_c::handleFIN4(tcp_sock* TCPSock)
{
  TCPSock->TCPState = TCP_CLOSED;
}


void TCPProtocol_c::handleSaveData(
  tcp_sock* TCPSock, uint32_t seq, uint32_t ack,
  uint16_t rwnd, char* TCPProtocolPacket, int TCPProtocolPacketLen
)
{
  // save the data
  TCPSock->ringBuffer.writeToBuffer(seq, TCPProtocolPacket, TCPProtocolPacketLen);

  // update log
  TCPSock->snd_una = ack;
  assert(TCPSock->ringBuffer.tail == TCPSock->rcv_nxt);
  while (TCPSock->ringBuffer.buf[TCPSock->ringBuffer.tail % TCPSock->ringBuffer.size]) {
    TCPSock->ringBuffer.tail++;
    TCPSock->rcv_nxt++;
  }
  TCPSock->rcv_wnd = rwnd;
}


void TCPProtocol_c::debug_printTCPSock(tcp_sock* TCPSock)
{
  printf("---------------TCPSock start---------------\n");
  printf("TCPState:           %d\n", TCPSock->TCPState);
  printf("localAddr:          0x%08x:0x%04x\n", TCPSock->localAddr.IP, TCPSock->localAddr.port);
  printf("peerAddr:           0x%08x:0x%04x\n", TCPSock->peerAddr.IP, TCPSock->peerAddr.port);
  printf("synRcvdList.size(): %lu\n", TCPSock->synRcvdList.size());
  printf("acceptList.size():  %lu\n", TCPSock->acceptList.size());
  printf("ringBuffer:         head %d; tail %d\n", TCPSock->ringBuffer.head, TCPSock->ringBuffer.tail);
  printf("snd_una:            %d\n", TCPSock->snd_una);
  printf("snd_nxt:            %d\n", TCPSock->snd_nxt);
  printf("rcv_nxt:            %d\n", TCPSock->rcv_nxt);
  printf("rcv_wnd:            %d\n", TCPSock->rcv_wnd);
  printf("^^^^^^^^^^^^^^^TCPSock end^^^^^^^^^^^^^^^\n");
}





