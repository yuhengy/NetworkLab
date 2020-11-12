#ifndef __IPPACKETMODULE_H__
#define __IPPACKETMODULE_H__

class etherPacketModule_c;
#include "routerTable.h"
#include <stdint.h>
#include <list>
#include <endian.h>

class IPPacketModule_c {
public:
  IPPacketModule_c();  //TODO erase this
  void addIPAddr(uint32_t IPAddr);
  void addEtherPacketModule(etherPacketModule_c* _etherPacketModule);
  void addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex
  );

  void readPacket(char* _packet, int _packetLen);
  void handleCurrentPacket();

  void debug_printCurrentPacketHeader();
  void debug_printIPList();
  void debug_printRouterTable();


private:
  // configuration
  std::list<uint32_t> IPList;
  etherPacketModule_c* etherPacketModule;

  // packet information
  char *packet;
  int packetLen;

  struct __attribute__ ((packed)) IPHeader_t {
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
