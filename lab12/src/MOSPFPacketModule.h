#ifndef __MOSPFPACKETMODULE_H__
#define __MOSPFPACKETMODULE_H__

class IPPacketModule_c;
#include <stdint.h>
#include <list>
#include <map>
#include <thread>
#include <mutex>

class MOSPFPacketModule_c {
public:
  void addIPAddr(uint32_t IPAddr);
  void addIPPacketModule(IPPacketModule_c* IPPacketModule);

  void startSubthread();
  void sendHelloThread();

  void handlePacket(char* MOSPFPacket, int MOSPFPacketLen, uint32_t srcIP);
  void sendPacket(
    uint8_t type, char* upLayerPacket, int upLayerPacketLen
  );

  void debug_printCurrentPacketHeader();
  void debug_printIPList();
  void debug_printneighbourInfoMap();

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
  void handleHello(
    char* helloPacket, int helloPacketLen, uint32_t rid, uint32_t srcIP
  );

  // hello packet
  struct __attribute__ ((packed)) helloContent_t {
    uint32_t mask;   // network mask associated with this interface
    uint16_t helloint; // number of seconds between hellos from this router
    uint16_t padding;  // set to zero
  };
  void debug_printHelloContent(struct helloContent_t content);

  // neighbour info
  struct neighbourInfo_t {
    uint32_t nbr_id;     // neighbor ID
    uint32_t nbr_ip;     // neighbor IP
    uint32_t nbr_mask;   // neighbor mask
    uint8_t  alive;      // alive for #(seconds)
  };
  std::map<uint32_t, struct neighbourInfo_t> neighbourInfoMap;
  std::mutex neighbourInfoMap_mutex;


  void handleLSU();
};



#endif
