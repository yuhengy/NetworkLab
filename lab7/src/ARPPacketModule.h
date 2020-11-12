#ifndef __ARPPACKETMODULE_H__
#define __ARPPACKETMODULE_H__

class etherPacketModule_c;
#include <stdint.h>
#include <map>

class ARPPacketModule_c {
public:
  void addIfaceIPToMac(uint32_t ifaceIP, uint64_t ifaceMac);
  void addEtherPacketModule(etherPacketModule_c* _etherPacketModule);

  void handlePacket(char* ARPPacket, int ARPPacketLen, int _ifaceIndex);
  void sendPacket(
    uint16_t arp_op,
    uint64_t arp_sha, uint32_t arp_spa, uint64_t arp_tha, uint32_t arp_tpa,
    char* upLayerPacket, int upLayerPacketLen, int _ifaceIndex
  );

  void debug_printCurrentPacketHeader();
  void debug_printMacList();


private:
  // configuration
  std::map<uint32_t, uint64_t> ifaceIPToMacMap;
  etherPacketModule_c* etherPacketModule;

  // header
  struct __attribute__ ((packed)) ARPHeader_t {
    uint16_t arp_hrd;    /* Format of hardware address.  */
    uint16_t arp_pro;    /* Format of protocol address.  */
    uint8_t  arp_hln;    /* Length of hardware address.  */
    uint8_t  arp_pln;    /* Length of protocol address.  */
    uint16_t arp_op;     /* ARP opcode (command).  */
    uint64_t arp_sha:48;  /* sender hardware address */
    uint32_t arp_spa;    /* sender protocol address */
    uint64_t arp_tha:48;  /* target hardware address */
    uint32_t arp_tpa;    /* target protocol address */
  } header;

  // handle packet in this layer
  void handleReq(char* ARPPacket, int ARPPacketLen, int ifaceIndex);
  void handleResp();

};



#endif
