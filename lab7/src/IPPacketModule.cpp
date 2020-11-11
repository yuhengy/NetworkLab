#include "IPPacketModule.h"

#include "endianSwap.h"
#include <stdio.h>

IPPacketModule_c::IPPacketModule_c()
{

}

void IPPacketModule_c::readIPPacket(char *packet, int len)
{
  IPHeader = *((struct IPHeader_t *)packet);
  endianSwap((uint8_t*)&(IPHeader.tot_len), 2);
  endianSwap((uint8_t*)&(IPHeader.id), 2);
  endianSwap((uint8_t*)&(IPHeader.frag_off), 2);
  endianSwap((uint8_t*)&(IPHeader.checksum), 2);
  endianSwap((uint8_t*)&(IPHeader.saddr), 4);
  endianSwap((uint8_t*)&(IPHeader.daddr), 4);
}

void IPPacketModule_c::handleCurrentIPPacket()
{
  debug_printCurrentPacket();
}

void IPPacketModule_c::debug_printCurrentPacket()
{
  printf("---------------IP Packet start---------------");
  printf("tos:      0x%2x\n", IPHeader.tos);
  printf("tot_len:  0x%4x\n", IPHeader.tot_len);
  printf("id:       0x%4x\n", IPHeader.id);
  printf("frag_off: 0x%4x\n", IPHeader.frag_off);
  printf("ttl:      0x%2x\n", IPHeader.ttl);
  printf("protocol: 0x%2x\n", IPHeader.protocol);
  printf("checksum: 0x%4x\n", IPHeader.checksum);
  printf("saddr:    0x%8x\n", IPHeader.saddr);
  printf("daddr:    0x%8x\n", IPHeader.daddr);
  printf("^^^^^^^^^^^^^^^IP Packet end^^^^^^^^^^^^^^^");
}
