#include "IPPacketModule.h"

#include "endianSwap.h"
#include "checksumBase.h"
#include "ICMPPacketModule.h"
#include <stdio.h>
#include <algorithm>

void IPPacketModule_c::addIPAddr(uint32_t IPAddr)
{
  IPList.push_back(IPAddr);
}

void IPPacketModule_c::addEtherPacketModule(etherPacketModule_c* _etherPacketModule)
{
  etherPacketModule = _etherPacketModule;
}

void IPPacketModule_c::addICMPPacketModule(ICMPPacketModule_c* _ICMPPacketModule)
{
  ICMPPacketModule = _ICMPPacketModule;
}

void IPPacketModule_c::addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex, uint32_t ifaceIP
  )
{
  routerTable.addRouterTableEntry(dest, mask, gw, ifaceIndex, ifaceIP);
}

//--------------------------------------------------------------------

void IPPacketModule_c::handlePacket(char* etherPacket, int etherPacketLen)
{
  header = *((struct IPHeader_t *)etherPacket);
  endianSwap((uint8_t*)&(header.tot_len) , 2);
  endianSwap((uint8_t*)&(header.id)      , 2);
  endianSwap((uint8_t*)&(header.frag_off), 2);
  endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.saddr)   , 4);
  endianSwap((uint8_t*)&(header.daddr)   , 4);


  printf("******************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket end*****\n");
  printf("****************************************************\n");


  uint32_t nextIP;
  int nextIfaceIndex;

  std::list<uint32_t>::iterator iter = find(IPList.begin(),IPList.end(),header.daddr);
  if (iter != IPList.end()) {
    switch (header.protocol) {
      case 0x01:
        printf("???????????TODO: upload to ICMP as normal\n");
        break;
      default:
        printf("ERROR: Unknown IPPacket type 0x%02x, ingore it.", header.protocol);
        break;
    }
  }
  else if (routerTable.findNextIPIface(header.daddr, &nextIP, &nextIfaceIndex)) {
    handleForward();
  }
  else {
    ICMPPacketModule->handlePacket(
      etherPacket + header.ihl * 4, etherPacketLen - header.ihl * 4,
      etherPacket, header.ihl * 4,
      0x03, 0x00
    );
  }
}

void IPPacketModule_c::sendPacket(
  uint8_t ttl, uint8_t protocol, uint32_t daddr,
  char* upLayerPacket, int upLayerPacketLen
)
{
  char* packet     = upLayerPacket    - sizeof(struct IPHeader_t);
  int packetLen  = upLayerPacketLen + sizeof(struct IPHeader_t);

  header.version  = 0x4;
  header.ihl      = 0x5;
  header.tos      = 0x00;
  header.tot_len  = packetLen;
  header.id       = 0x0000;
  header.frag_off = 0x4000;
  header.ttl      = ttl;
  header.protocol = protocol;
  header.checksum = 0x0000;
  header.daddr    = daddr;


  //header.saddr    = saddr;
  //uint64_t targetMac = ;
  printf("???????????TODO: look arp cache\n");

  *((struct IPHeader_t *)packet) = header;
  endianSwap(((uint8_t*)packet) + 2 , 2);
  endianSwap(((uint8_t*)packet) + 4 , 2);
  endianSwap(((uint8_t*)packet) + 6 , 2);
  endianSwap(((uint8_t*)packet) + 12, 4);
  endianSwap(((uint8_t*)packet) + 16, 4);

  header.checksum = checksumBase((uint16_t *)packet, 20, 0);
  ((struct IPHeader_t *)packet)->checksum = header.checksum;
  endianSwap(((uint8_t*)packet) + 10, 2);



  printf("\n\n");
  printf("******************************************************\n");
  printf("**********IPPacketModule_c::sendPacket start**********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("**********IPPacketModule_c::sendPacket end**********\n");
  printf("****************************************************\n");



/*
  etherPacketModule->sendPacket(
    targetMac, 0x0800,
    packet, packetLen, ifaceIndex
  );
*/
}


//--------------------------------------------------------------------

void IPPacketModule_c::handleForward()
{
  printf("???????????TODO: handleForward\n");
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
  printf("---------------IP IPList start---------------\n");
  for (std::list<uint32_t>::iterator iter =
    IPList.begin(); iter != IPList.end(); iter++){

    printf("IPAddr: 0x%08x\n", *iter);
  }
  printf("^^^^^^^^^^^^^^^IP IPList end^^^^^^^^^^^^^^^\n");
}


void IPPacketModule_c::debug_printRouterTable()
{
  routerTable.debug_printRouterTable();
}
