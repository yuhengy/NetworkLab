#ifndef __ARPMISSPENDINGBUFF_H__
#define __ARPMISSPENDINGBUFF_H__

#include <stdint.h>
#include <time.h>
#include <map>
#include <list>


// TODO: I am not sure whether this module is necessary
//       Since it is so messy
struct ARPMissPendingEntry_c {
  uint8_t  ttl;
  uint8_t  protocol;
  uint32_t saddr;
  uint32_t daddr;
  uint8_t  ihl;
  char*    upLayerPacket;
  int      upLayerPacketLen;
  time_t   addedTime;
};

class ARPMissPendingBuff_c {
public:
  void addARPMissPendingBuffEntry(
    uint32_t nextIP,
    uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
    char* upLayerPacket, int upLayerPacketLen
  );
  std::list<struct ARPMissPendingEntry_c*>* getARPMissPendingList(uint32_t IP);
  struct ARPMissPendingEntry_c* getTimeoutEntry();

  void debug_printARPMissPendingBuff();




private:

  std::map<uint32_t, std::list<struct ARPMissPendingEntry_c*>> ARPMissPendingBuff;

};

#endif


