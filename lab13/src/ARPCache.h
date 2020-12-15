#ifndef __NEWARPCACHE_H__
#define __NEWARPCACHE_H__

#include <stdint.h>
#include <time.h>
#include <map>


class ARPCache_c {
public:
  void addARPCacheEntry(uint32_t IP, uint64_t mac);
  bool findMac(uint32_t IP, uint64_t* mac);

  void debug_printARPCache();




private:
  struct ARPCacheEntry_t {
    uint32_t IP;
    uint64_t mac;
    time_t addedTime;
  };

  std::map<uint32_t, struct ARPCacheEntry_t*> ARPCache;

};

#endif
