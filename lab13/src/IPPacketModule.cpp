#include "IPPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "etherPacketModule.h"
#include "ARPPacketModule.h"
#include "ICMPPacketModule.h"
#include "MOSPFPacketModule.h"
#include "TCPPacketModule.h"
#include <stdio.h>

#define NEIGHBOUR_BROARDCAST_IP 0xe0000005  // 224.0.0.5
#define NEIGHBOUR_BROARDCAST_MAC 0x01005e000005  //01:00:5E:00:00:05

void IPPacketModule_c::addIPToIfaceIndexMap(uint32_t IPAddr, int ifaceIndex)
{
  IPToIfaceIndexMap[IPAddr] = ifaceIndex;
}

void IPPacketModule_c::addEtherPacketModule(etherPacketModule_c* _etherPacketModule)
{
  etherPacketModule = _etherPacketModule;
}

void IPPacketModule_c::addARPPacketModule(ARPPacketModule_c* _ARPPacketModule)
{
  ARPPacketModule = _ARPPacketModule;
}

void IPPacketModule_c::addICMPPacketModule(ICMPPacketModule_c* _ICMPPacketModule)
{
  ICMPPacketModule = _ICMPPacketModule;
}

void IPPacketModule_c::addMOSPFPacketModule(MOSPFPacketModule_c* _MOSPFPacketModule)
{
  MOSPFPacketModule = _MOSPFPacketModule;
}

void IPPacketModule_c::addRouterTable(routerTable_c* _routerTable)
{
  routerTable = _routerTable;
}

//--------------------------------------------------------------------

void IPPacketModule_c::handlePacket(char* IPPacket, int IPPacketLen, uint32_t ifaceIP)
{
  header = *((struct IPHeader_t *)IPPacket);
  endianSwap((uint8_t*)&(header.tot_len) , 2);
  endianSwap((uint8_t*)&(header.id)      , 2);
  endianSwap((uint8_t*)&(header.frag_off), 2);
  endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.saddr)   , 4);
  endianSwap((uint8_t*)&(header.daddr)   , 4);


#if 1
  printf("******************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("******IPPacketModule_c::handleCurrentPacket end*****\n");
  printf("****************************************************\n");
#endif

  auto iter0 = TCPPacketModuleList.begin();
  for (; iter0 != TCPPacketModuleList.end(); iter0++) {

    if (ifaceIP == (*iter0)->iface->getIP()) {
      if (header.ihl != 0x5) {
        printf("Error: think more when IP ihl != 5.\n");
      }
      if (!(*iter0)->handlePacket(
        IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4,
        header.saddr, header.daddr
      )) {
        
        ICMPPacketModule->handlePacket(
          IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4, header.saddr,
          IPPacket, header.ihl * 4,
          0x03, 0x01
        );
      }
      break;
    }
  }
  if (iter0 == TCPPacketModuleList.end()) {
    printf("Error: IP Packet is not send to TCP.\n");
  }
  return;


  std::map<uint32_t, int>::iterator iter = IPToIfaceIndexMap.find(header.daddr);

  if (header.ttl == 1) {
    ICMPPacketModule->handlePacket(
      IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4, header.saddr,
      IPPacket, header.ihl * 4,
      0x0b, 0x00
    );
  }
  
  else if (header.daddr == NEIGHBOUR_BROARDCAST_IP) {
    MOSPFPacketModule->handlePacket(
      IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4,
      header.saddr, ifaceIP
    );
  }
  
  else if (iter != IPToIfaceIndexMap.end()) {
    switch (header.protocol) {
      case 0x01:
        ICMPPacketModule->handlePacket(
          IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4, header.saddr,
          IPPacket, header.ihl * 4,
          0x00, 0x00
        );
        break;
      default:
        printf("ERROR: Unknown IPPacket type 0x%02x, ingore it.", header.protocol);
        break;
    }
  }
  
  else if (routerTable->hasNextIP(header.daddr)) {
    handleForward(IPPacket, IPPacketLen);
  }
  
  else {
    // TODO: a better structure should solve thiss in `sendPacket`
    ICMPPacketModule->handlePacket(
      IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4, header.saddr,
      IPPacket, header.ihl * 4,
      0x03, 0x00
    );
  }
}

void IPPacketModule_c::handleARPPacket(uint32_t IP, uint64_t mac)
{
  ARPCache.addARPCacheEntry(IP, mac);

  //ARPMissPendingBuff.debug_printARPMissPendingBuff();

  
  std::list<struct ARPMissPendingEntry_c*>* ARPMissPendingList =
    ARPMissPendingBuff.getARPMissPendingList(IP);

  if (ARPMissPendingList != NULL) {
    for (std::list<struct ARPMissPendingEntry_c*>::iterator iter =
      ARPMissPendingList->begin(); iter != ARPMissPendingList->end(); iter++){

      sendPacket(
        (*iter)->ttl, (*iter)->protocol, (*iter)->saddr, (*iter)->daddr, (*iter)->ihl,
        (*iter)->upLayerPacket, (*iter)->upLayerPacketLen
      );
    }
  }

}

void IPPacketModule_c::sendPacket(
  uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
  char* upLayerPacket, int upLayerPacketLen
)
{
  char* packet     = upLayerPacket    - ihl * 4;
  int packetLen  = upLayerPacketLen + ihl * 4;

  // TODO: use a new struct to fill
  header.version  = 0x4;
  header.ihl      = ihl;
  header.tos      = 0x00;
  header.tot_len  = packetLen;
  header.id       = 0x0000;
  header.frag_off = 0x4000;
  if (ttl == 0) {
    header.ttl = ((struct IPHeader_t *)packet)->ttl;
    header.ttl--;
  }
  else {
    header.ttl      = ttl;
  }
  if (protocol == 0) {
    header.protocol = ((struct IPHeader_t *)packet)->protocol;
  }
  else {
    header.protocol = protocol;
  }
  header.checksum = 0x0000;
  header.daddr    = daddr;


  int ifaceIndex;
  uint64_t targetMac;

  if (daddr == NEIGHBOUR_BROARDCAST_IP) {
  findNeighbourBroadcastIPMacIface(saddr, &ifaceIndex, &targetMac);
  }
  else {
    bool succeed = findNormalIPMacIface(
      ttl, protocol, saddr, daddr, ihl,
      upLayerPacket, upLayerPacketLen,
      &ifaceIndex, &targetMac  // this two output
    );
    if (!succeed) return;
  }


  *((struct IPHeader_t *)packet) = header;
  endianSwap(((uint8_t*)packet) + 2 , 2);
  endianSwap(((uint8_t*)packet) + 4 , 2);
  endianSwap(((uint8_t*)packet) + 6 , 2);
  endianSwap(((uint8_t*)packet) + 12, 4);
  endianSwap(((uint8_t*)packet) + 16, 4);

  header.checksum = checksumBase((uint16_t *)packet, 20, 0);
  ((struct IPHeader_t *)packet)->checksum = header.checksum;
  //endianSwap(((uint8_t*)packet) + 10, 2);



#if 0
  printf("\n\n");
  printf("******************************************************\n");
  printf("**********IPPacketModule_c::sendPacket start**********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("**********IPPacketModule_c::sendPacket end**********\n");
  printf("****************************************************\n");
#endif

  etherPacketModule->sendPacket(
    targetMac, 0x0800,
    packet, packetLen, ifaceIndex
  );

}


//--------------------------------------------------------------------

void IPPacketModule_c::handleForward(char* IPPacket, int IPPacketLen)
{
  sendPacket(
    header.ttl - 1, header.protocol, header.saddr, header.daddr, header.ihl,
    IPPacket + header.ihl * 4, IPPacketLen - header.ihl * 4
  );
}

// TODO: this should not be a handle* private function
// move this into sendPacket for code structure consistence
void IPPacketModule_c::handleARPCacheMiss(
  uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
  char* upLayerPacket, int upLayerPacketLen,
  uint32_t nextIP, int ifaceIndex
)
{
  char* packet = (char*)malloc(ETHER_HEADER_LEN + ARP_HEADER_LEN);
  ARPPacketModule->sendPacket(
    0x0001, header.saddr, 0xffffffffffff, nextIP,
    packet + ETHER_HEADER_LEN + ARP_HEADER_LEN, 0, ifaceIndex
  );

  ARPMissPendingBuff.addARPMissPendingBuffEntry(
    nextIP,
    ttl, protocol, saddr, daddr, ihl,
    upLayerPacket, upLayerPacketLen
  );

}

//--------------------------------------------------------------------

bool IPPacketModule_c::findNormalIPMacIface(
  uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
  char* upLayerPacket, int upLayerPacketLen,
  int* ifaceIndex, uint64_t* targetMac  // this two output
)
{
  // STEP1 ask routerTable
  uint32_t nextIP;
  uint32_t ifaceIP;

  if (!routerTable->findNextIP(daddr, &nextIP, ifaceIndex, &ifaceIP)) {
    printf("Error: IPModule unable to send this packet\n");
  }

  printf("routerTable_c::findNextIP: destIP %x; nextIP %x; ifaceIndex %d, ifaceIP %x\n",
    daddr, nextIP, *ifaceIndex, ifaceIP);

  // TODO: a little messy
  if (saddr == 0) {
    header.saddr = ifaceIP;
  }
  else {
    header.saddr = saddr;
  }

  // STEP2 ask arpCache
  if (!ARPCache.findMac(nextIP, targetMac)) {
    handleARPCacheMiss(
      ttl, protocol, saddr, daddr, ihl, upLayerPacket, upLayerPacketLen,
      nextIP, *ifaceIndex
    );
    return false;
  }

  return true;
}


void IPPacketModule_c::findNeighbourBroadcastIPMacIface(
  uint32_t saddr, int* ifaceIndex, uint64_t* targetMac
)
{
  header.saddr = saddr;
  std::map<uint32_t, int>::iterator iter = IPToIfaceIndexMap.find(saddr);
  if (iter == IPToIfaceIndexMap.end()) {
    printf("Error: IP is asked to broadcast from unknow source IP\n");
  }
  *ifaceIndex = iter->second;
  *targetMac = NEIGHBOUR_BROARDCAST_MAC;
}


//--------------------------------------------------------------------

// TODO this funciton is very bad
void IPPacketModule_c::sweepARPMissPendingBuff()
{
  struct ARPMissPendingEntry_c* entry;
  while ((entry = ARPMissPendingBuff.getTimeoutEntry()) != NULL) {
    printf("----> sweep a ARPMissPendingEntry\n");
    ICMPPacketModule->handlePacket(
      entry->upLayerPacket, entry->upLayerPacketLen, entry->saddr,
      entry->upLayerPacket - 20, 20,
      0x03, 0x01
    );
  }

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

void IPPacketModule_c::debug_printIPToIfaceIndexMap()
{
  printf("---------------IP IPToIfaceIndexMap start---------------\n");
  for (std::map<uint32_t, int>::iterator iter = IPToIfaceIndexMap.begin();
    iter != IPToIfaceIndexMap.end(); iter++){
    printf("IP: 0x%08x --> ifaceIndex: 0x%d\n", iter->first, iter->second);
  }
  printf("^^^^^^^^^^^^^^^IP IPToIfaceIndexMap end^^^^^^^^^^^^^^^\n");
}
