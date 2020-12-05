#ifndef __MOSPFPACKETMODULE_H__
#define __MOSPFPACKETMODULE_H__

class IPPacketModule_c;
#include "routerTable.h"
#include <stdint.h>
#include <vector> 
#include <list>
#include <map>
#include <thread>
#include <mutex>

class MOSPFPacketModule_c {
public:
  void addIPAddr(uint32_t IPAddr);
  void addRouterTable(routerTable_c* _routerTable);
  void addIPPacketModule(IPPacketModule_c* IPPacketModule);

  void startSubthread();

  void handlePacket(
    char* MOSPFPacket, int MOSPFPacketLen, uint32_t srcIP, uint32_t ifaceIP
  );
  void sendPacket(
    uint8_t type, char* upLayerPacket, int upLayerPacketLen,
    uint32_t rid, uint32_t exclueIfaceIP
  );

  void debug_printIPList();
  void debug_printNeighbourInfoMap();
  void debug_printNodeInfoMap();

  ~MOSPFPacketModule_c();


private:
  // configuration
  std::list<uint32_t> IPList;
  routerTable_c* routerTable;
  IPPacketModule_c* IPPacketModule;

  // sub threads
  void sendHelloThread();
  void sendLSUThread();
  void neighbourTimeoutThread();
  void nodeTimeoutThread();
  std::thread hello, LSU, neighbourTimeout, nodeTimeout;

  // header
  struct __attribute__ ((packed)) MOSPFHeader_t {
    uint8_t  version;
    uint8_t  type;
    uint16_t len;
    uint32_t rid;
    uint32_t aid;
    uint16_t checksum;
    uint16_t padding;
  };
  void debug_printCurrentPacketHeader(struct MOSPFHeader_t header);


  // handle hello packet in this layer
  void handleHello(
    char* helloPacket, int helloPacketLen, uint32_t rid, uint32_t srcIP
  );

  // hello content
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
  bool neighbourInfoMap_changed = false;


  // handle LSU packet in this layer
  void handleLSU(
    char* LSUPacket, int LSUPacketLen, uint32_t rid, uint32_t ifaceIP
  );

  // LSU content
  uint16_t LSUSequence = 0;
  struct __attribute__ ((packed)) LSU1Content_t {
    uint16_t seq;
    uint8_t  ttl;
    uint8_t  unused;
    uint32_t nadv;
  };
  struct __attribute__ ((packed)) LSU2Content_t {
    uint32_t network;
    uint32_t mask;
    uint32_t rid;
  };
  void debug_printLSUContent(
    struct LSU1Content_t content1, std::list<struct LSU2Content_t> content2
  );

  // database
  struct nodeInfo_t {
    uint32_t rid;
    uint16_t seq;
    uint32_t nadv;
    uint8_t  alive;
    std::list<struct LSU2Content_t> content2;
  };
  std::map<uint32_t, struct nodeInfo_t> nodeInfoMap;
  std::mutex nodeInfoMap_mutex;


  // update routerTable
  void updateRouterTable();

  // messy fix
  std::mutex IPServe_mutex;


};



#endif
