#include "routerTable.h"

#include <stdio.h>

void routerTable_c::addRouterTableEntry(
    uint32_t dest, uint32_t mask, uint32_t gw, int ifaceIndex
  )
{
  routerTableEntry_t* routerTableEntry = new routerTableEntry_t;
  routerTableEntry->dest       = dest;
  routerTableEntry->mask       = mask;
  routerTableEntry->gw         = gw;
  routerTableEntry->ifaceIndex = ifaceIndex;
  routerTable.push_back(routerTableEntry);
}



bool routerTable_c::findNextIPIface(
  uint32_t destIP, uint32_t* nextIP, int* nextIfaceIndex
)
{
  struct routerTableEntry_t* maxMatchRouterTableEntry;
  uint32_t maxMatchLength = 0;

  for (std::list<struct routerTableEntry_t*>::iterator iter =
    routerTable.begin(); iter != routerTable.end(); iter++){

    if ((destIP & (*iter)->mask) == (destIP & (*iter)->mask)) {
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
    *nextIfaceIndex = maxMatchRouterTableEntry->ifaceIndex;
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
    printf("ifaceIndex: %d\n", (*iter)->ifaceIndex);
  }
  printf("^^^^^^^^^^^^^^^IP routerTable end^^^^^^^^^^^^^^^\n");
}
