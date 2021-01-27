#include "routerTable.h"

#include <stdio.h>

void routerTable_c::addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex, uint32_t ifaceIP
  )
{
  struct routerTableEntry_t* routerTableEntry = new struct routerTableEntry_t;
  routerTableEntry->dest       = dest;
  routerTableEntry->mask       = mask;
  routerTableEntry->gw         = gw;
  routerTableEntry->ifaceIndex = ifaceIndex;
  routerTableEntry->ifaceIP    = ifaceIP;
  routerTable.push_back(routerTableEntry);
}

void routerTable_c::clearMOSPFEntry()
{
  while(MOSPFEntryNum > 0) {
    routerTable.pop_back();
    MOSPFEntryNum--;
  }
}

void routerTable_c::addMOSPFEntry(uint32_t dest, uint32_t mask, uint32_t nextNet)
{
  printf("Dijkstra: dest: %x, nextNet: %x\n", dest, nextNet);

  uint32_t nextIP;
  int      ifaceIndex;
  uint32_t ifaceIP;

  // This is a little messy
  // TODO: solve when each iface map to one MOSPFModule
  if (!findNextIP(nextNet, &nextIP, &ifaceIndex, &ifaceIP)){
    printf("Error: cannot add MOSPF entry to router table.\n");
  }
  addRouterTableEntry(dest, mask, nextIP, ifaceIndex, ifaceIP);
  MOSPFEntryNum++;
}

bool routerTable_c::hasNextIP(uint32_t destIP)
{

  for (std::list<struct routerTableEntry_t*>::iterator iter =
    routerTable.begin(); iter != routerTable.end(); iter++){

    if ((destIP & (*iter)->mask) == ((*iter)->dest & (*iter)->mask)) {
      return true;
    }
  }

  return false;

}

bool routerTable_c::findNextIP(
  uint32_t destIP, uint32_t* nextIP, int* ifaceIndex, uint32_t* ifaceIP
)
{
  struct routerTableEntry_t* maxMatchRouterTableEntry;
  uint32_t maxMatchLength = 0;

  for (std::list<struct routerTableEntry_t*>::iterator iter =
    routerTable.begin(); iter != routerTable.end(); iter++){

    if ((destIP & (*iter)->mask) == ((*iter)->dest & (*iter)->mask)) {
      if ((*iter)->mask > maxMatchLength) {
        maxMatchLength = (*iter)->mask;
        maxMatchRouterTableEntry = (*iter);
      }
    }
  }

  if (maxMatchLength != 0) {
    if (maxMatchRouterTableEntry->gw != 0) {
      *nextIP = maxMatchRouterTableEntry->gw;
    }
    else {
      *nextIP = destIP;
    }
    *ifaceIndex = maxMatchRouterTableEntry->ifaceIndex;
    *ifaceIP    = maxMatchRouterTableEntry->ifaceIP;
    return true;
  }
  else {
    return false;
  }
}

void routerTable_c::debug_printRouterTable()
{
  printf("---------------IP routerTable start---------------\n");
  for (std::list<struct routerTableEntry_t*>::iterator iter =
    routerTable.begin(); iter != routerTable.end(); iter++){

    printf("dest: 0x%08x  ", (*iter)->dest);
    printf("mask: 0x%08x  ", (*iter)->mask);
    printf("gw: 0x%08x  ", (*iter)->gw);
    printf("ifaceIndex: %d  ", (*iter)->ifaceIndex);
    printf("ifaceIP: 0x%08x\n", (*iter)->ifaceIP);
  }
  printf("^^^^^^^^^^^^^^^IP routerTable end^^^^^^^^^^^^^^^\n");
}
