#ifndef __ROUTERTABLE_H__
#define __ROUTERTABLE_H__

#include <stdint.h>
#include <list>
#include "iface.h"


class routerTable_c {
public:
  void addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex, uint32_t ifaceIP
  );
  bool hasNextIP(uint32_t destIP);
  bool findNextIP(
    uint32_t destIP, uint32_t* nextIP, int* ifaceIndex, uint32_t* ifaceIP
  );

  void debug_printRouterTable();




private:
  struct routerTableEntry_t {
    uint32_t dest;       // destination ip address (could be network or host)
    uint32_t mask;       // network mask of dest
    uint32_t gw;         // ip address of next hop (will be 0 if dest is in 
                // the same network with iface)
    int ifaceIndex;
    uint32_t ifaceIP;
  };

  std::list<struct routerTableEntry_t*> routerTable;

};

#endif
