#ifndef __IPPACKETMODULE_H__
#define __IPPACKETMODULE_H__

class etherPacketModule_c;
class ICMPPacketModule_c;
#include "routerTable.h"
#include <stdint.h>
#include <list>
#include <endian.h>

class IPPacketModule_c {
public:
  void addIPAddr(uint32_t IPAddr);
  void addEtherPacketModule(etherPacketModule_c* _etherPacketModule);
  void addICMPPacketModule(ICMPPacketModule_c* _ICMPPacketModule);
  void addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex, uint32_t ifaceIP
  );

  void handlePacket(char* etherPacket, int etherPacketLen);
  void sendPacket(
    uint8_t ttl, uint8_t protocol, uint32_t daddr,
    char* upLayerPacket, int upLayerPacketLen
  );

  void debug_printCurrentPacketHeader();
  void debug_printIPList();
  void debug_printRouterTable();


private:
  // configuration
  std::list<uint32_t> IPList;
  etherPacketModule_c* etherPacketModule;
  ICMPPacketModule_c* ICMPPacketModule;

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
  } header;

  // router table
  routerTable_c routerTable;



  // handle packet in this layer
  void handleForward();


};



#endif
