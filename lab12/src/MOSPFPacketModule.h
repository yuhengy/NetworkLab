#ifndef __MOSPFPACKETMODULE_H__
#define __MOSPFPACKETMODULE_H__

class IPPacketModule_c;
#include <stdint.h>
#include <list>
#include <thread>

class MOSPFPacketModule_c {
public:
  void addIPAddr(uint32_t IPAddr);
  void addIPPacketModule(IPPacketModule_c* IPPacketModule);

  void startSubthread();
  void sendHelloThread();

  void handlePacket(char* MOSPFPacket, int MOSPFPacketLen);
  void sendPacket(
    uint8_t type, char* upLayerPacket, int upLayerPacketLen
  );

  void debug_printCurrentPacketHeader();
  void debug_printIPList();

  ~MOSPFPacketModule_c();


private:
  // configuration
  std::list<uint32_t> IPList;
  IPPacketModule_c* IPPacketModule;

  // sub threads
  std::thread hello;

  // header
  struct __attribute__ ((packed)) MOSPFHeader_t {
    uint8_t  version;
    uint8_t  type;
    uint16_t len;
    uint32_t rid;
    uint32_t aid;
    uint16_t checksum;
    uint16_t padding;
  } header;

  



  // handle packet in this layer
  void handleHello();
  void handleLSU();


};



#endif
