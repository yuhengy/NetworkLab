#include "routerTable.h"
#include "etherPacketModule.h"
#include "ARPPacketModule.h"
#include "IPPacketModule.h"
#include "ICMPPacketModule.h"
#include "MOSPFPacketModule.h"
#include "iface.h"
#include "endianSwap.h"
#include "messyOldFramework/ether.h"
#include "messyOldFramework/main.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <poll.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>

#include <map>

routerTable_c* routerTable;

etherPacketModule_c* etherPacketModule;
ARPPacketModule_c*   ARPPacketModule;
IPPacketModule_c*    IPPacketModule;
ICMPPacketModule_c*  ICMPPacketModule;
MOSPFPacketModule_c* MOSPFPacketModule;




// handle packet, hand the packet to handle_ip_packet or handle_arp_packet
// according to ether_type
void handle_packet(iface_info_t *iface, char *packet, int len)
{
  etherPacketModule->handlePacket(packet, len, iface->index);

}


void initIfaceMacIPConfig()
{
  iface_info_t *iface = NULL;
  uint64_t mac;
  list_for_each_entry(iface, &instance->iface_list, list) {
    mac = 0;
    memcpy(((char*)(&(mac))) + 2, iface->mac, 6);
    endianSwap((uint8_t*)&mac, 8);

    etherPacketModule->addIface(iface->index, 
      new iface_c(iface->fd, iface->index, mac, iface->ip, iface->mask, iface->name, iface->ip_str));
    ARPPacketModule->addIfaceIPToMac(iface->ip, mac);
    IPPacketModule->addIPToIfaceIndexMap(iface->ip, iface->index);

    if (iface->mask != 0xffffff00) {
      printf("Error: MOSPF does not support mask != 0xffffff00\n");
    }
    MOSPFPacketModule->addIPAddr(iface->ip);
  }

  printf("\n\n");
  printf("**********************************************\n");
  printf("*********init initNetworkConfig start*********\n");
  printf("**********************************************\n");
  etherPacketModule->debug_printIfaceMap();
  ARPPacketModule->debug_printMacList();
  IPPacketModule->debug_printIPToIfaceIndexMap();
  MOSPFPacketModule->debug_printIPList();
  printf("********************************************\n");
  printf("*********init initNetworkConfig end*********\n");
  printf("********************************************\n");
}

void initRouterTable()
{
  rt_entry_t *entry = NULL;
  list_for_each_entry(entry, &rtable, list) {
    routerTable->addRouterTableEntry(
      entry->dest, entry->mask, entry->gw, entry->iface->index, entry->iface->ip
    );
  }

  printf("\n\n");
  printf("**********************************************\n");
  printf("**********init initRouterTable start**********\n");
  printf("**********************************************\n");
  routerTable->debug_printRouterTable();
  printf("********************************************\n");
  printf("**********init initRouterTable end**********\n");
  printf("********************************************\n");
}


int main(int argc, const char **argv)
{
  if (getuid() && geteuid()) {
    printf("Permission denied, should be superuser!\n");
    exit(1);
  }
  init_ustack();
  routerTable = new routerTable_c();

  etherPacketModule = new etherPacketModule_c();
  ARPPacketModule   = new ARPPacketModule_c();
  IPPacketModule    = new IPPacketModule_c();
  ICMPPacketModule  = new ICMPPacketModule_c();
  MOSPFPacketModule = new MOSPFPacketModule_c();

  initIfaceMacIPConfig();
  initRouterTable();

  etherPacketModule->addARPPacketModule(ARPPacketModule);
  etherPacketModule->addIPPacketModule(IPPacketModule);

  ARPPacketModule->addEtherPacketModule(etherPacketModule);
  ARPPacketModule->addIPPacketModule(IPPacketModule);

  IPPacketModule->addRouterTable(routerTable);
  IPPacketModule->addEtherPacketModule(etherPacketModule);
  IPPacketModule->addARPPacketModule(ARPPacketModule);
  IPPacketModule->addICMPPacketModule(ICMPPacketModule);
  IPPacketModule->addMOSPFPacketModule(MOSPFPacketModule);

  ICMPPacketModule->addIPPacketModule(IPPacketModule);

  MOSPFPacketModule->addRouterTable(routerTable);
  MOSPFPacketModule->addIPPacketModule(IPPacketModule);


  MOSPFPacketModule->startSubthread();


  ustack_run();

  return 0;
}
