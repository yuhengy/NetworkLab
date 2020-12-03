#ifndef __ETHERPACKETMODULE_H__
#define __ETHERPACKETMODULE_H__

#include "iface.h"
class ARPPacketModule_c;
class IPPacketModule_c;
#include <stdint.h>
#include <map>

class etherPacketModule_c {
public:
  void addIface(int index, iface_c* iface);
  void addARPPacketModule(ARPPacketModule_c* _ARPPacketModule);
  void addIPPacketModule(IPPacketModule_c* _IPPacketModule);

  void handlePacket(char* etherPacket, int etherPacketLen, int ifaceIndex);
  void sendPacket(
    uint64_t ether_dhost, uint16_t ether_type,
    char* upLayerPacket, int upLayerPacketLen, int ifaceIndex
  );

  void debug_printCurrentPacketHeader();
  void debug_printIfaceMap();

private:
  // configuration
  std::map<int, iface_c*> ifaceMap;
  ARPPacketModule_c* ARPPacketModule;
  IPPacketModule_c* IPPacketModule;

  // header
  struct __attribute__ ((packed)) etherHeader_t {
    uint64_t ether_dhost:48;
    uint64_t ether_shost:48;
    uint16_t ether_type;
  } header;

};



#endif
