#ifndef __ETHERPACKETMODULE_H__
#define __ETHERPACKETMODULE_H__

#include "iface.h"
class ARPPacketModule_c;
class IPPacketModule_c;
#include <stdint.h>
#include <map>

class etherPacketModule_c {
public:
  etherPacketModule_c();
  void addIface(int index, iface_c* iface);
  void addARPPacketModule(ARPPacketModule_c* _ARPPacketModule);
  void addIPPacketModule(IPPacketModule_c* _IPPacketModule);

  void readPacket(char* etherPacket, int etherPacketLen, int _ifaceIndex);
  void handleCurrentPacket();
  void writePacket(
    uint64_t ether_dhost, uint64_t ether_shost, uint16_t ether_type,
    char* IPARPPacket, int IPARPPacketLen, int _ifaceIndex
  );
  void sendPacket();

  void debug_printCurrentPacketHeader();
  void debug_printIfaceMap();

private:
  std::map<int, iface_c*> ifaceMap;
  ARPPacketModule_c* ARPPacketModule;
  IPPacketModule_c* IPPacketModule;

  char* packet;
  int packetLen;
  int ifaceIndex;

  struct __attribute__ ((packed)) etherHeader_t {
    uint64_t ether_dhost:48;
    uint64_t ether_shost:48;
    uint16_t ether_type;
  } header;

};



#endif
