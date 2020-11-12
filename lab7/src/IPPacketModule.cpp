#include "IPPacketModule.h"

#include "endianSwap.h"
#include <stdio.h>

IPPacketModule_c::IPPacketModule_c()
{

}

void IPPacketModule_c::addIPAddr(uint32_t IPAddr)
{
  IPList.push_back(IPAddr);
}

void IPPacketModule_c::addEtherPacketModule(etherPacketModule_c* _etherPacketModule)
{
  etherPacketModule = _etherPacketModule;
}

//--------------------------------------------------------------------

void IPPacketModule_c::readPacket(char *packet, int len)
{
  header = *((struct IPHeader_t *)packet);
  endianSwap((uint8_t*)&(header.tot_len) , 2);
  endianSwap((uint8_t*)&(header.id)      , 2);
  endianSwap((uint8_t*)&(header.frag_off), 2);
  endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.saddr)   , 4);
  endianSwap((uint8_t*)&(header.daddr)   , 4);
}

void IPPacketModule_c::handleCurrentPacket()
{
  printf("******************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket end*****\n");
  printf("****************************************************\n");
}

//--------------------------------------------------------------------

void IPPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------IP Packet start---------------\n");
  printf("version:  0x%01x\n", header.version);
  printf("ihl:      0x%01x\n", header.ihl);
  printf("tos:      0x%02x\n", header.tos);
  printf("tot_len:  0x%04x\n", header.tot_len);
  printf("id:       0x%04x\n", header.id);
  printf("frag_off: 0x%04x\n", header.frag_off);
  printf("ttl:      0x%02x\n", header.ttl);
  printf("protocol: 0x%02x\n", header.protocol);
  printf("checksum: 0x%04x\n", header.checksum);
  printf("saddr:    0x%08x\n", header.saddr);
  printf("daddr:    0x%08x\n", header.daddr);
  printf("^^^^^^^^^^^^^^^IP Packet end^^^^^^^^^^^^^^^\n");
}

void IPPacketModule_c::debug_printIPList()
{
  printf("---------------ARP IPList start---------------\n");
  for (std::list<uint32_t>::iterator iter = IPList.begin();
    iter != IPList.end(); iter++){
    printf("IPAddr: 0x%08x\n", *iter);
  }
  printf("^^^^^^^^^^^^^^^ARP IPList end^^^^^^^^^^^^^^^\n");
}
