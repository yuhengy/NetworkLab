#include "ARPMissPendingBuff.h"

#include <stdio.h>

void ARPMissPendingBuff_c::addARPMissPendingBuffEntry(
    uint32_t nextIP,
    uint8_t ttl, uint8_t protocol, uint32_t saddr, uint32_t daddr, uint8_t ihl,
    char* upLayerPacket, int upLayerPacketLen
  )
{
  struct ARPMissPendingEntry_c* ARPMissPendingEntry = new struct ARPMissPendingEntry_c;
  ARPMissPendingEntry->ttl = ttl;
  ARPMissPendingEntry->protocol = protocol;
  ARPMissPendingEntry->saddr = saddr;
  ARPMissPendingEntry->daddr = daddr;
  ARPMissPendingEntry->ihl = ihl;
  ARPMissPendingEntry->upLayerPacket = upLayerPacket;
  ARPMissPendingEntry->upLayerPacketLen = upLayerPacketLen;
  ARPMissPendingEntry->addedTime = time(NULL);

  if (ARPMissPendingBuff.find(nextIP) == ARPMissPendingBuff.end()) {
    // TODO: is this assign right?
    std::list<struct ARPMissPendingEntry_c*> ARPMissPendingList;
    ARPMissPendingBuff[nextIP] = ARPMissPendingList;
  }

  ARPMissPendingBuff.find(nextIP)->second.push_back(ARPMissPendingEntry);
}

std::list<struct ARPMissPendingEntry_c*>* ARPMissPendingBuff_c::getARPMissPendingList(uint32_t IP)
{
  std::map<uint32_t, std::list<struct ARPMissPendingEntry_c*>>::iterator iter =
    ARPMissPendingBuff.find(IP);

  if (iter != ARPMissPendingBuff.end()) {
    std::list<struct ARPMissPendingEntry_c*>* ARPMissPendingList =
      new std::list<struct ARPMissPendingEntry_c*>(iter->second);

    ARPMissPendingBuff.erase(iter);

    return ARPMissPendingList;  // TODO: can this simpler ?
  }
  else {
    return NULL;
  }
}

struct ARPMissPendingEntry_c* ARPMissPendingBuff_c::getTimeoutEntry()
{
  struct ARPMissPendingEntry_c* ARPMissPendingEntry = new struct ARPMissPendingEntry_c;
  for (std::map<uint32_t, std::list<struct ARPMissPendingEntry_c*>>::iterator iter1 =
    ARPMissPendingBuff.begin(); iter1 != ARPMissPendingBuff.end(); iter1++){

    //printf("\n---> pending at IP: 0x%08x\n", iter1->first);

    for (std::list<struct ARPMissPendingEntry_c*>::iterator iter2 =
      iter1->second.begin(); iter2 != iter1->second.end(); iter2++){

      if (time(NULL) - (*iter2)->addedTime > 5) {
        *ARPMissPendingEntry = **iter2;
        iter1->second.erase(iter2);
        return ARPMissPendingEntry;
      }
    }
  }
  return NULL;

}

void ARPMissPendingBuff_c::debug_printARPMissPendingBuff()
{
  printf("---------------IP ARPMissPendingBuff start---------------\n");
  for (std::map<uint32_t, std::list<struct ARPMissPendingEntry_c*>>::iterator iter1 =
    ARPMissPendingBuff.begin(); iter1 != ARPMissPendingBuff.end(); iter1++){

    printf("\n---> pending at IP: 0x%08x\n", iter1->first);

    for (std::list<struct ARPMissPendingEntry_c*>::iterator iter2 =
      iter1->second.begin(); iter2 != iter1->second.end(); iter2++){

      printf("ttl:              0x%02x\n", (*iter2)->ttl);
      printf("protocol:         0x%02x\n", (*iter2)->protocol);
      printf("saddr:            0x%08x\n", (*iter2)->saddr);
      printf("daddr:            0x%08x\n", (*iter2)->daddr);
      printf("ihl:              0x%02x\n", (*iter2)->ihl);
      printf("upLayerPacket:    0x%p\n"  , (*iter2)->upLayerPacket);
      printf("upLayerPacketLen: %d\n"    , (*iter2)->upLayerPacketLen);
      printf("addedTime:        %ld\n"   , (*iter2)->addedTime);
    }
  }
  printf("^^^^^^^^^^^^^^^IP ARPMissPendingBuff end^^^^^^^^^^^^^^^\n");
}
