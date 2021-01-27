#include "etherPacketModule.h"

#include "endianSwap.h"
#include "IPPacketModule.h"
#include "ARPPacketModule.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

void etherPacketModule_c::addIface(int index, iface_c* iface)
{
  ifaceMap[index] = iface;
}

void etherPacketModule_c::addARPPacketModule(ARPPacketModule_c* _ARPPacketModule)
{
  ARPPacketModule = _ARPPacketModule;
}

void etherPacketModule_c::addIPPacketModule(IPPacketModule_c* _IPPacketModule)
{
  IPPacketModule = _IPPacketModule;
}

//--------------------------------------------------------------------

void etherPacketModule_c::handlePacket(char* etherPacket, int etherPacketLen, int ifaceIndex)
{
  header = *((struct etherHeader_t *)etherPacket);
  endianSwap((uint8_t*)&(header)            , 6);
  endianSwap(((uint8_t*)&(header)) + 6      , 6);
  endianSwap((uint8_t*)&(header.ether_type) , 2);


#if 0
  printf("\n\n");
  printf("******************************************************\n");
  printf("****etherPacketModule_c::handleCurrentPacket start****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("****etherPacketModule_c::handleCurrentPacket end****\n");
  printf("****************************************************\n");
#endif

  switch (header.ether_type) {
    case 0x0800:
      IPPacketModule->handlePacket(
        etherPacket + sizeof(struct etherHeader_t),
        etherPacketLen - sizeof(struct etherHeader_t),
        ifaceMap[ifaceIndex]->getIP()
      );
      break;
    case 0x0806:
      ARPPacketModule->handlePacket(
        etherPacket + sizeof(struct etherHeader_t),
        etherPacketLen - sizeof(struct etherHeader_t),
        ifaceIndex
      );
      break;
    default:
      printf("ERROR: Unknown etherPacket type 0x%04x, ingore it.", header.ether_type);
      break;
  }
}

void etherPacketModule_c::sendPacket(
    uint64_t ether_dhost, uint16_t ether_type,
    char* upLayerPacket, int upLayerPacketLen, int ifaceIndex
  )
{
  char* etherPacket    = upLayerPacket    - sizeof(struct etherHeader_t);
  int   etherPacketLen = upLayerPacketLen + sizeof(struct etherHeader_t);

  header.ether_dhost = ether_dhost;
  header.ether_shost = ifaceMap.find(ifaceIndex)->second->getMac();
  header.ether_type  = ether_type;
  *((struct etherHeader_t *)etherPacket) = header;
  endianSwap( (uint8_t*)etherPacket      , 6);
  endianSwap(((uint8_t*)etherPacket) + 6 , 6);
  endianSwap(((uint8_t*)etherPacket) + 12, 2);



#if 0
  printf("\n\n");
  printf("******************************************************\n");
  printf("*********etherPacketModule_c::sendPacket start********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("*********etherPacketModule_c::sendPacket end********\n");
  printf("****************************************************\n");
#endif

  struct sockaddr_ll addr;
  memset(&addr, 0, sizeof(struct sockaddr_ll));
  addr.sll_family = AF_PACKET;
  addr.sll_ifindex = ifaceIndex;
  addr.sll_halen = 6;
  addr.sll_protocol = htons(header.ether_type);
  memcpy(addr.sll_addr, &header, 6);

  if (sendto(ifaceMap.find(ifaceIndex)->second->getFd(), etherPacket, etherPacketLen, 0, (const struct sockaddr *)&addr,
        sizeof(struct sockaddr_ll)) < 0) {
    printf("Error: Send raw packet failed");
  }
  free(etherPacket);
}

//--------------------------------------------------------------------

void etherPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------ether Packet start---------------\n");
  printf("ether_dhost: 0x%012lx\n" , header.ether_dhost);
  printf("ether_shost: 0x%012lx\n" , header.ether_shost);
  printf("ether_type:  0x%02x\n"   , header.ether_type);
  printf("^^^^^^^^^^^^^^^ether Packet end^^^^^^^^^^^^^^^\n");
}

void etherPacketModule_c::debug_printIfaceMap()
{
  printf("---------------ether ifaceMap start---------------\n");
  for (std::map<int, iface_c*>::iterator iter = ifaceMap.begin();
    iter != ifaceMap.end(); iter++){
      printf("-->fd = %d\n", iter->first);
      iter->second->debug_printiface();
  }
  printf("^^^^^^^^^^^^^^^ether ifaceMap end^^^^^^^^^^^^^^^\n");
}
