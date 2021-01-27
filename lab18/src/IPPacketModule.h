#ifndef __IPPACKETMODULE_H__
#define __IPPACKETMODULE_H__

class etherPacketModule_c;
class ARPPacketModule_c;
class ICMPPacketModule_c;
class MOSPFPacketModule_c;
class TCPPacketModule_c;
#include "routerTable.h"
#include "ARPCache.h"
#include "ARPMissPendingBuff.h"
#include <stdint.h>
#include <vector>
#include <map>
#include <endian.h>

class IPPacketModule_c {
public:
  void addIPToIfaceIndexMap(uint32_t IPAddr, int ifaceIndex);
  void addRouterTable(routerTable_c* _routerTable);
  void addEtherPacketModule(etherPacketModule_c* _etherPacketModule);
  void addARPPacketModule(ARPPacketModule_c* _ARPPacketModule);
  void addICMPPacketModule(ICMPPacketModule_c* _ICMPPacketModule);
  void addMOSPFPacketModule(MOSPFPacketModule_c* _MOSPFPacketModule);
  void addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule) {
    TCPPacketModuleList.push_back(_TCPPacketModule);
  }

  void handlePacket(char* IPPacket, int IPPacketLen, uint32_t ifaceIP);
  void handleARPPacket(uint32_t IP, uint64_t mac);
  void sendPacket(
    uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
    char* upLayerPacket, int upLayerPacketLen
  );

  // header
  struct __attribute__ ((packed)) IPHeader_t {  //TODO header length can change
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t  ihl:4;
    uint8_t  version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t  version:4;
    uint8_t  ihl:4;
#endif
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t saddr;
    uint32_t daddr;
  };

  void sweepARPMissPendingBuff();

  void debug_printCurrentPacketHeader(IPHeader_t header);
  void debug_printIPToIfaceIndexMap();


private:
  // configuration
  std::map<uint32_t, int> IPToIfaceIndexMap;
  etherPacketModule_c* etherPacketModule;
  ARPPacketModule_c* ARPPacketModule;
  ICMPPacketModule_c* ICMPPacketModule;
  MOSPFPacketModule_c* MOSPFPacketModule;
  std::vector<TCPPacketModule_c*> TCPPacketModuleList;

  // sub modules
  routerTable_c* routerTable;
  ARPCache_c ARPCache;
  ARPMissPendingBuff_c ARPMissPendingBuff;
  



  // handle packet in this layer
  void handleForward(char* IPPacket, int IPPacketLen, IPHeader_t header);
  void handleARPCacheMiss(
    uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
    char* upLayerPacket, int upLayerPacketLen,
    uint32_t nextIP, int ifaceIndex,
    IPHeader_t header
  );

  // different methods to send packet
  bool findNormalIPMacIface(
    uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
    char* upLayerPacket, int upLayerPacketLen,
    int* ifaceIndex, uint64_t* targetMac,  // this two output
    IPHeader_t* header
  );
  void findNeighbourBroadcastIPMacIface(
    uint32_t saddr, int* ifaceIndex, uint64_t* targetMac, IPHeader_t* header
  );


};



#endif
