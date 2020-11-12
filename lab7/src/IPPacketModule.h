#ifndef __IPPACKETMODULE_H__
#define __IPPACKETMODULE_H__

class etherPacketModule_c;
#include <stdint.h>
#include <list>

class IPPacketModule_c {
public:
  IPPacketModule_c();
  void addIPAddr(uint32_t IPAddr);
  void addEtherPacketModule(etherPacketModule_c* _etherPacketModule);

  void readPacket(char* _packet, int _packetLen);
  void handleCurrentPacket();

  void debug_printCurrentPacketHeader();
  void debug_printIPList();


private:
  std::list<uint32_t> IPList;
  etherPacketModule_c* etherPacketModule;

  char *packet;
  int packetLen;

  struct __attribute__ ((packed)) IPHeader_t {
    uint8_t  version:4;
    uint8_t  ihl:4;
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

};



#endif
