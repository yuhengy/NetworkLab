#include "routerTable.h"
#include "nat.h"
#include "etherPacketModule.h"
#include "ARPPacketModule.h"
#include "IPPacketModule.h"
#include "ICMPPacketModule.h"
#include "MOSPFPacketModule.h"
#include "TCPPacketModule.h"
#include "TCPSock.h"
#include "TCPApp.h"
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
#include <vector>

std::vector<iface_c*> ifaceList;
routerTable_c* routerTable;
nat_c* nat;

etherPacketModule_c* etherPacketModule;
ARPPacketModule_c*   ARPPacketModule;
IPPacketModule_c*    IPPacketModule;
ICMPPacketModule_c*  ICMPPacketModule;
MOSPFPacketModule_c* MOSPFPacketModule;
std::vector<TCPPacketModule_c> TCPPacketModuleList;
TCPSock_c* TCPSock;
TCPApp_c* TCPApp;




// handle packet, hand the packet to handle_ip_packet or handle_arp_packet
// according to ether_type
void handle_packet(iface_info_t *iface, char *packet, int len)
{
  etherPacketModule->handlePacket(packet, len, iface->index);

}


void initIfaceMacIPConfig()
{
  iface_info_t *iface = NULL;
  int ifaceID = 0;
  uint64_t mac;
  list_for_each_entry(iface, &instance->iface_list, list) {
    mac = 0;
    memcpy(((char*)(&(mac))) + 2, iface->mac, 6);
    endianSwap((uint8_t*)&mac, 8);

    ifaceList.push_back(new iface_c(
      iface->fd, iface->index, mac, iface->ip, iface->mask, iface->name, iface->ip_str
    ));
    etherPacketModule->addIface(iface->index, *(ifaceList.end() - 1));
    ARPPacketModule->addIfaceIPToMac(iface->ip, mac);
    IPPacketModule->addIPToIfaceIndexMap(iface->ip, iface->index);
    TCPPacketModuleList.push_back(TCPPacketModule_c(*(ifaceList.end() - 1)));

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

#if 0
  nat = new nat_c(argv[1]);
  printf("\n\n");
  printf("**********************************************\n");
  printf("****************init nat start****************\n");
  printf("**********************************************\n");
  nat->debug_printRuleList();
  printf("**********************************************\n");
  printf("*****************init nat end*****************\n");
  printf("**********************************************\n");
#endif

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
  for (auto iter = TCPPacketModuleList.begin();
    iter != TCPPacketModuleList.end(); iter++) {

#if 0
    nat->addTCPPacketModule(&(*iter));
#endif
    IPPacketModule->addTCPPacketModule(&(*iter));
    iter->addIPPacketModule(IPPacketModule);
#if 0
    iter->addNat(nat);
#endif
  }

  ICMPPacketModule->addIPPacketModule(IPPacketModule);

  MOSPFPacketModule->addRouterTable(routerTable);
  MOSPFPacketModule->addIPPacketModule(IPPacketModule);

#if 0
  MOSPFPacketModule->startSubthread();
#endif

  if (ifaceList.size() != 1) {
    printf("Error: TCP only support 1 iface.\n");
  }
  TCPSock = new TCPSock_c(*ifaceList.begin());
  TCPApp = new TCPApp_c();

  TCPSock->addTCPPacketModule(&(*TCPPacketModuleList.begin()));
  TCPSock->addTCPApp(TCPApp);
  TCPApp->addTCPSock(TCPSock);

  if (strcmp(argv[1], "server") == 0) {
    int temp;
    uint16_t port;
    sscanf(argv[2], "%d", &temp); port = (uint16_t)temp;
    TCPApp->startServerthread(&port);
  }
  else if (strcmp(argv[1], "client") == 0) {
    int temp;
    struct sock_addr skaddr;
    sscanf(argv[2], "%x", &(skaddr.IP));
    sscanf(argv[3], "%d", &temp); skaddr.port = (uint16_t)temp;
    TCPApp->startClientthread(&skaddr);
  }
  else {
    printf("Error: wrong command formate.\n");
  }


  ustack_run();

  return 0;
}
