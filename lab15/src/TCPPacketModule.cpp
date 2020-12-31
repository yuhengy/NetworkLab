#include "TCPPacketModule.h"


#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "IPPacketModule.h"
#include "TCPSock.h"
#include <string.h>


# define TCP_FIN  0x01
# define TCP_SYN  0x02
# define TCP_RST  0x04
# define TCP_PSH  0x08
# define TCP_ACK  0x10
# define TCP_URG  0x20

#define TCP_PROTOCAL 6


bool TCPPacketModule_c::handlePacket(
  char* TCPPacket, int TCPPacketLen, uint32_t sIP, uint32_t dIP
)
{
  struct TCPHeader_t header = *((struct TCPHeader_t *)TCPPacket);
  endianSwap((uint8_t*)&(header.sport), 2);
  endianSwap((uint8_t*)&(header.dport), 2);
  endianSwap((uint8_t*)&(header.seq)  , 4);
  endianSwap((uint8_t*)&(header.ack)  , 4);
  endianSwap((uint8_t*)&(header.rwnd) , 2);
  // checksum always compute from packet
  // and same endian in packet and header
  //endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.urp)  , 2);

  if (header.off != 5) {
    printf("Error: TCP not support vary head length.\n");
  }

#if 1
  printf("******************************************************\n");
  printf("*********TCPPacketModule_c::handlePacket start********\n");
  printf("******************************************************\n");
  debug_printPacketHeader(header);
  printf("****************************************************\n");
  printf("*********TCPPacketModule_c::handlePacket end********\n");
  printf("****************************************************\n");
#endif

#if 0
  return nat->translate(
    this, TCPPacket + TCP_HEADER_LEN, TCPPacketLen - TCP_HEADER_LEN,
    sIP, header.sport, dIP, header.dport
  );
#endif

  TCPSock->handlePacket(
    TCPPacket + TCP_HEADER_LEN, TCPPacketLen - TCP_HEADER_LEN,
    sIP, header.sport,
    header.seq, header.ack, header.flags, header.rwnd
  );
  return true;

}




void TCPPacketModule_c::sendPacket(
  char* upLayerPacket, int upLayerPacketLen,
  uint32_t sIP, uint16_t sPort, uint32_t dIP, uint16_t dPort,
  uint8_t off, uint8_t ttl
)
{
  char* packet  = upLayerPacket    - TCP_HEADER_LEN;
  int packetLen = upLayerPacketLen + TCP_HEADER_LEN;

  struct TCPHeader_t header;
  header.sport = sPort;
  header.dport = dPort;
  header.seq = ((struct TCPHeader_t *)packet)->seq;
  endianSwap((uint8_t*)&(header.seq)  , 4);
  header.ack = ((struct TCPHeader_t *)packet)->ack;
  endianSwap((uint8_t*)&(header.ack)  , 4);
  header.x2 = ((struct TCPHeader_t *)packet)->x2;
  if (off == 0)
    header.off = ((struct TCPHeader_t *)packet)->off;
  else
    header.off = off;
  header.flags = ((struct TCPHeader_t *)packet)->flags;
  header.rwnd = ((struct TCPHeader_t *)packet)->rwnd;
  endianSwap((uint8_t*)&(header.rwnd) , 2);
  header.checksum = 0x0000;
  header.urp = ((struct TCPHeader_t *)packet)->urp;

  *((struct TCPHeader_t *)packet) = header;
  endianSwap(((uint8_t*)packet)    , 2);
  endianSwap(((uint8_t*)packet) + 2, 2);
  endianSwap(((uint8_t*)packet) + 4, 4);
  endianSwap(((uint8_t*)packet) + 8, 4);
  endianSwap(((uint8_t*)packet) + 14, 2);
  endianSwap(((uint8_t*)packet) + 18, 2);

  char* checksumTemp = new char[packetLen + 12];
  memcpy(checksumTemp+12, packet, packetLen);
  *(uint32_t*)(checksumTemp    ) = sIP;
  *(uint32_t*)(checksumTemp + 4) = dIP;
  *(uint8_t*) (checksumTemp + 8) = 0;
  *(uint8_t*) (checksumTemp + 9) = TCP_PROTOCAL;
  *(uint16_t*)(checksumTemp + 10) = packetLen;
  endianSwap(((uint8_t*)checksumTemp     ), 4);
  endianSwap(((uint8_t*)checksumTemp +  4), 4);
  endianSwap(((uint8_t*)checksumTemp + 10), 2);
  header.checksum = checksumBase((uint16_t *)checksumTemp, packetLen + 12, 0);
  free(checksumTemp);
  ((struct TCPHeader_t *)packet)->checksum = header.checksum;


#if 1
  printf("******************************************************\n");
  printf("*********TCPPacketModule_c::sendPacket start********\n");
  printf("******************************************************\n");
  debug_printPacketHeader(header);
  printf("****************************************************\n");
  printf("*********TCPPacketModule_c::sendPacket end********\n");
  printf("****************************************************\n");
#endif
  
  IPPacketModule->sendPacket(ttl, TCP_PROTOCAL, sIP, dIP, 0x5, packet, packetLen);
}




//--------------------------------------------------------------------

void TCPPacketModule_c::debug_printPacketHeader(struct TCPHeader_t header)
{
  printf("---------------TCP Packet Header start---------------\n");
  printf("sport:    0x%04x\n", header.sport);
  printf("dport:    0x%04x\n", header.dport);
  printf("seq:      0x%08x\n", header.seq);
  printf("ack:      0x%08x\n", header.ack);
  printf("x2:       0x%01x\n", header.x2);
  printf("off:      0x%01x\n", header.off);
  printf("flags:    0x%02x\n", header.flags);
  printf("rwnd:     0x%04x\n", header.rwnd);
  printf("checksum: 0x%04x\n", header.checksum);
  printf("urp:      0x%04x\n", header.urp);
  printf("^^^^^^^^^^^^^^^TCP Packet Header end^^^^^^^^^^^^^^^\n");
}






