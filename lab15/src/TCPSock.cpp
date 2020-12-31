#include "TCPSock.h"

#include "common.h"
#include "TCPPacketModule.h"
#include <unistd.h>


void TCPSock_c::handlePacket(
  char* TCPSockPacket, int TCPSockPacketLen,
  uint32_t sIP, uint16_t sPort,
  uint32_t seq, uint32_t ack, uint8_t flags, uint16_t rwnd
)
{

#if 1
  printf("******************************************************\n");
  printf("*************TCPSock_c::handlePacket start************\n");
  printf("******************************************************\n");
  printf("****************************************************\n");
  printf("*************TCPSock_c::handlePacket end************\n");
  printf("****************************************************\n");
#endif

}


struct tcp_sock* TCPSock_c::alloc_tcp_sock()
{
  return NULL;
}


int TCPSock_c::tcp_sock_bind(struct tcp_sock* tsk, struct sock_addr* sockAddr)
{
  sleep(3);
  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    iface->getIP(), sockAddr->port, 0x0a000002, 0xffff,
    5, TTL_INIT
  );
  return 0;
}


int TCPSock_c::tcp_sock_listen(struct tcp_sock *tsk, int backlog)
{
  return 0;
}


struct tcp_sock* TCPSock_c::tcp_sock_accept(struct tcp_sock *tsk)
{
  return NULL;
}


int TCPSock_c::tcp_sock_connect(struct tcp_sock *tsk, struct sock_addr *skaddr)
{
  sleep(3);
  char *emptyPacket = (char*)malloc(
    ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN
  );
  emptyPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + TCP_HEADER_LEN;
  TCPPacketModule->sendPacket(
    emptyPacket, 0,
    iface->getIP(), 0xffff, skaddr->IP, skaddr->port,
    5, TTL_INIT
  );
  return 0;
}


void TCPSock_c::tcp_sock_close(struct tcp_sock *tsk)
{

}






