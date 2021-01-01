#include "TCPApp.h"

#include "TCPProtocol.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
///*
// tcp server application, listens to port (specified by arg) and serves only one
// connection request
void TCPApp_c::TCPServer(void *arg)
{
  sleep(1);
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

  char rbuf[1001];
  char wbuf[1024];
  int rlen = 0;
  while (1) {
    rlen = TCPProtocol->tcp_sock_read(csk, rbuf, 1000);
    if (rlen == 0) {
      printf("tcp_sock_read return 0, finish transmission.\n");
      break;
    } 
    else if (rlen > 0) {
      rbuf[rlen] = '\0';
      sprintf(wbuf, "server echoes: %s", rbuf);
      if (TCPProtocol->tcp_sock_write(csk, wbuf, strlen(wbuf)) < 0) {
        printf("tcp_sock_write return negative value, something goes wrong.\n");
        exit(1);
      }
    }
    else {
      printf("tcp_sock_read return negative value, something goes wrong.\n");
      exit(1);
    }
  }

  printf("close this connection.");

  TCPProtocol->tcp_sock_close(csk);
  
  return NULL;
}

// tcp client application, connects to server (ip:port specified by arg), each
// time sends one bulk of data and receives one bulk of data 
void TCPApp_c::TCPClient(void *arg)
{
  sleep(2);
  struct sock_addr* skaddr = (struct sock_addr* )arg;

  struct tcp_sock *tsk = TCPProtocol->alloc_tcp_sock();

  if (TCPProtocol->tcp_sock_connect(tsk, skaddr) < 0) {
    printf("tcp_sock connect to server (0x%08x:0x%02x)failed.\n", skaddr->IP, skaddr->port);
    exit(1);
  }

  char *wbuf = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int wlen = strlen(wbuf);
  char rbuf[1001];
  int rlen = 0;

  int n = 10;
  for (int i = 0; i < n; i++) {
    if (TCPProtocol->tcp_sock_write(tsk, wbuf + i, wlen - n) < 0)
      break;

    rlen = TCPProtocol->tcp_sock_read(tsk, rbuf, 1000);
    if (rlen == 0) {
      printf("tcp_sock_read return 0, finish transmission.\n");
      break;
    }
    else if (rlen > 0) {
      rbuf[rlen] = '\0';
      fprintf(stdout, "%s\n", rbuf);
    }
    else {
      printf("tcp_sock_read return negative value, something goes wrong.\n");
      exit(1);
    }
    sleep(1);
  }

  TCPProtocol->tcp_sock_close(tsk);

  return NULL;
}
//*/

/*

// tcp server application, listens to port (specified by arg) and serves only one
// connection request
void TCPApp_c::TCPServer(void *arg)
{
  sleep(1);
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



  FILE *fp = NULL;
  char buff[255];
 
  fp = fopen("/tmp/test.txt", "r");
  fscanf(fp, "%s", buff)

  char rbuf[1001];
  char wbuf[1024];
  int rlen = 0;
  while (1) {
    rlen = TCPProtocol->tcp_sock_read(csk, rbuf, 1000);
    if (rlen == 0) {
      printf("tcp_sock_read return 0, finish transmission.\n");
      break;
    } 
    else if (rlen > 0) {
      rbuf[rlen] = '\0';
      sprintf(wbuf, "server echoes: %s", rbuf);
      if (TCPProtocol->tcp_sock_write(csk, wbuf, strlen(wbuf)) < 0) {
        printf("tcp_sock_write return negative value, something goes wrong.\n");
        exit(1);
      }
    }
    else {
      printf("tcp_sock_read return negative value, something goes wrong.\n");
      exit(1);
    }
  }

  printf("close this connection.");

  TCPProtocol->tcp_sock_close(csk);
  
  return NULL;
}

// tcp client application, connects to server (ip:port specified by arg), each
// time sends one bulk of data and receives one bulk of data 
void TCPApp_c::TCPClient(void *arg)
{
  sleep(2);
  struct sock_addr* skaddr = (struct sock_addr* )arg;

  struct tcp_sock *tsk = TCPProtocol->alloc_tcp_sock();

  if (TCPProtocol->tcp_sock_connect(tsk, skaddr) < 0) {
    printf("tcp_sock connect to server (0x%08x:0x%02x)failed.\n", skaddr->IP, skaddr->port);
    exit(1);
  }

  char *wbuf = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int wlen = strlen(wbuf);
  char rbuf[1001];
  int rlen = 0;

  int n = 10;
  for (int i = 0; i < n; i++) {
    if (TCPProtocol->tcp_sock_write(tsk, wbuf + i, wlen - n) < 0)
      break;

    rlen = TCPProtocol->tcp_sock_read(tsk, rbuf, 1000);
    if (rlen == 0) {
      printf("tcp_sock_read return 0, finish transmission.\n");
      break;
    }
    else if (rlen > 0) {
      rbuf[rlen] = '\0';
      fprintf(stdout, "%s\n", rbuf);
    }
    else {
      printf("tcp_sock_read return negative value, something goes wrong.\n");
      exit(1);
    }
    sleep(1);
  }

  TCPProtocol->tcp_sock_close(tsk);

  return NULL;
}
*/
