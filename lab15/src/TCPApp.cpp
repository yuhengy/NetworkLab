#include "TCPApp.h"

#include "TCPProtocol.h"
#include <unistd.h>


void TCPApp_c::TCPServer(void *arg)
{
  uint16_t port = *(uint16_t *)arg;
  struct tcp_sock *tsk = TCPProtocol->alloc_tcp_sock();

  struct sock_addr addr;
  addr.IP = 0;
  addr.port = port;
  if (TCPProtocol->tcp_sock_bind(tsk, &addr) < 0) {
    printf("tcp_sock bind to port 0x%02x failed\n", port);
    exit(1);
  }

  if (TCPProtocol->tcp_sock_listen(tsk, 3) < 0) {
    printf("tcp_sock listen failed\n");
    exit(1);
  }

  printf("listen to port %hu.\n", port);

  struct tcp_sock *csk = TCPProtocol->tcp_sock_accept(tsk);

  printf("accept a connection.\n");

  sleep(5);

  TCPProtocol->tcp_sock_close(csk);
  
  return NULL;
  
}


void TCPApp_c::TCPClient(void *arg)
{
  struct sock_addr* skaddr = (struct sock_addr* )arg;

  struct tcp_sock *tsk = TCPProtocol->alloc_tcp_sock();

  if (TCPProtocol->tcp_sock_connect(tsk, skaddr) < 0) {
    printf("tcp_sock connect to server (0x%08x:0x%02x)failed.\n", skaddr->IP, skaddr->port);
    exit(1);
  }

  sleep(1);

  TCPProtocol->tcp_sock_close(tsk);

  return NULL;
  
}
