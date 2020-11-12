#include "ARPPacketModule.h"

#include "endianSwap.h"
#include "etherPacketModule.h"
#include <stdio.h>

ARPPacketModule_c::ARPPacketModule_c()
{

}

void ARPPacketModule_c::addIfaceIPToMac(uint32_t ifaceIP, uint64_t ifaceMac)
{
  ifaceIPToMacMap[ifaceIP] = ifaceMac;
}

void ARPPacketModule_c::addEtherPacketModule(etherPacketModule_c* _etherPacketModule)
{
  etherPacketModule = _etherPacketModule;
}

//--------------------------------------------------------------------

void ARPPacketModule_c::readPacket(char* ARPPacket, int ARPPacketLen, int _ifaceIndex)
{
  packet     = ARPPacket;
  packetLen  = ARPPacketLen;
  ifaceIndex = _ifaceIndex;

  header = *((struct ARPHeader_t *)packet);
  endianSwap((uint8_t*)&(header.arp_hrd), 2);
  endianSwap((uint8_t*)&(header.arp_pro), 2);
  endianSwap((uint8_t*)&(header.arp_op) , 2);
  endianSwap(((uint8_t*)&(header.arp_op))  + 2, 6);
  endianSwap((uint8_t*)&(header.arp_spa), 4);
  endianSwap(((uint8_t*)&(header.arp_spa)) + 4, 6);
  endianSwap((uint8_t*)&(header.arp_tpa), 4);
}

void ARPPacketModule_c::handleCurrentPacket()
{
  printf("\n\n");
  printf("****************************************************\n");
  printf("****ARPPacketModule_c::handleCurrentPacket start****\n");
  printf("****************************************************\n");
  debug_printCurrentPacketHeader();
  printf("**************************************************\n");
  printf("****ARPPacketModule_c::handleCurrentPacket end****\n");
  printf("**************************************************\n");

  switch (header.arp_op) {
    case 0x0001:
      handleReq();  break;
    case 0x0002:
      handleResp(); break;
    default:
      printf("ERROR: Unknown ARPPacket type 0x%04x, ingore it.", header.arp_op);
      break;
  }
}

void ARPPacketModule_c::writePacket(
  uint16_t arp_op,
  uint64_t arp_sha, uint32_t arp_spa, uint64_t arp_tha, uint32_t arp_tpa,
  char* applicationPacket, int applicationPacketLen, int _ifaceIndex
)
{
  packet     = applicationPacket    - sizeof(struct ARPHeader_t);
  packetLen  = applicationPacketLen + sizeof(struct ARPHeader_t);
  ifaceIndex = _ifaceIndex;

  header.arp_hrd = 0x0001;
  header.arp_pro = 0x0800;
  header.arp_hln = 0x06;
  header.arp_pln = 0x04;
  header.arp_op  = arp_op;
  header.arp_sha = arp_sha;
  header.arp_spa = arp_spa;
  header.arp_tha = arp_tha;
  header.arp_tpa = arp_tpa;
  (*((struct ARPHeader_t *)packet)) = header;
  endianSwap( (uint8_t*)packet      , 2);
  endianSwap(((uint8_t*)packet) + 2 , 2);
  endianSwap(((uint8_t*)packet) + 6 , 2);
  endianSwap(((uint8_t*)packet) + 8 , 6);
  endianSwap(((uint8_t*)packet) + 14, 4);
  endianSwap(((uint8_t*)packet) + 18, 6);
  endianSwap(((uint8_t*)packet) + 24, 4);
}

void ARPPacketModule_c::sendPacket()
{
  printf("\n\n");
  printf("******************************************************\n");
  printf("**********ARPPacketModule_c::sendPacket start*********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("**********ARPPacketModule_c::sendPacket end*********\n");
  printf("****************************************************\n");

  etherPacketModule->writePacket(
    header.arp_tha, header.arp_sha, 0x0806,
    packet, packetLen, ifaceIndex
  );
  etherPacketModule->sendPacket();
}

//--------------------------------------------------------------------

void ARPPacketModule_c::handleReq()
{
  std::map<uint32_t, uint64_t>::iterator iter = ifaceIPToMacMap.find(header.arp_tpa);
  if(iter != ifaceIPToMacMap.end()) {
    writePacket(
      0x0002,
      iter->second, header.arp_tpa, header.arp_sha, header.arp_spa,
      packet + sizeof(struct ARPHeader_t),
      packetLen - sizeof(struct ARPHeader_t),
      ifaceIndex
    );
    sendPacket();
  }
}

void ARPPacketModule_c::handleResp()
{

}

//--------------------------------------------------------------------

void ARPPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------ARP Packet start---------------\n");
  printf("arp_hrd: 0x%04x\n"  , header.arp_hrd);
  printf("arp_pro: 0x%04x\n"  , header.arp_pro);
  printf("arp_hln: 0x%02x\n"  , header.arp_hln);
  printf("arp_pln: 0x%02x\n"  , header.arp_pln);
  printf("arp_op:  0x%04x\n"  , header.arp_op);
  printf("arp_sha: 0x%012lx\n", header.arp_sha);
  printf("arp_spa: 0x%08x\n"  , header.arp_spa);
  printf("arp_tha: 0x%012lx\n", header.arp_tha);
  printf("arp_tpa: 0x%08x\n"  , header.arp_tpa);
  printf("^^^^^^^^^^^^^^^ARP Packet end^^^^^^^^^^^^^^^\n");
}

void ARPPacketModule_c::debug_printMacList()
{
  printf("---------------ARP macList start---------------\n");
  for (std::map<uint32_t, uint64_t>::iterator iter = ifaceIPToMacMap.begin();
    iter != ifaceIPToMacMap.end(); iter++){
    printf("ifaceIP: 0x%08x --> ifaceMac: 0x%012lx\n", iter->first, iter->second);
  }
  printf("^^^^^^^^^^^^^^^ARP macList end^^^^^^^^^^^^^^^\n");
}
