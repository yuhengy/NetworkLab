#include "ARPCache.h"

#include <stdio.h>

void ARPCache_c::addARPCacheEntry(uint32_t IP, uint64_t mac)
{
  struct ARPCacheEntry_t* ARPCacheEntry = new struct ARPCacheEntry_t;
  ARPCacheEntry->IP        = IP;
  ARPCacheEntry->mac       = mac;
  ARPCacheEntry->addedTime = time(NULL);;
  ARPCache[IP] = ARPCacheEntry;
}

bool ARPCache_c::findMac(uint32_t IP, uint64_t* mac)
{
  std::map<uint32_t, struct ARPCacheEntry_t*>::iterator iter =
    ARPCache.find(IP);

  if (iter != ARPCache.end()) {
    *mac = iter->second->mac;
    return true;
  }
  else {
    return false;
  }
}

void ARPCache_c::debug_printARPCache()
{
  printf("---------------IP ARPCache start---------------\n");
  for (std::map<uint32_t, struct ARPCacheEntry_t*>::iterator iter =
    ARPCache.begin(); iter != ARPCache.end(); iter++){

    printf("IP: 0x%08x  ", iter->second->IP);
    printf("mac: 0x%012lx  ", iter->second->mac);
    printf("addedTime: %ld\n", iter->second->addedTime);
  }
  printf("^^^^^^^^^^^^^^^IP ARPCache end^^^^^^^^^^^^^^^\n");
}
