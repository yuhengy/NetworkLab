#include "IPPacketModule.h"
#include "messyOldFramework/ether.h"
#include "messyOldFramework/main.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>

IPPacketModule_c* IPPacketModule;



// handle packet, hand the packet to handle_ip_packet or handle_arp_packet
// according to ether_type
void handle_packet(iface_info_t *iface, char *packet, int len)
{
  struct ether_header *eh = (struct ether_header *)packet;

  // log(DEBUG, "got packet from %s, %d bytes, proto: 0x%04hx\n", 
  //    iface->name, len, ntohs(eh->ether_type));
  switch (ntohs(eh->ether_type)) {
    case ETH_P_IP:
      IPPacketModule->readIPPacket(packet + ETHER_HDR_SIZE, len);
      IPPacketModule->handleCurrentIPPacket();
      break;
    case ETH_P_ARP:
      handle_arp_packet(iface, packet + ETHER_HDR_SIZE, len);
      break;
    default:
      log(ERROR, "Unknown packet type 0x%04hx, ingore it.", \
          ntohs(eh->ether_type));
      break;
  }
}





int main(int argc, const char **argv)
{
  if (getuid() && geteuid()) {
    printf("Permission denied, should be superuser!\n");
    exit(1);
  }
  init_ustack();
  IPPacketModule = new IPPacketModule_c();

  ustack_run();

  return 0;
}
