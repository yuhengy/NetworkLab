#include "ICMPPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include <stdio.h>
#include <algorithm>

void ICMPPacketModule_c::addIPPacketModule(IPPacketModule_c* _IPPacketModule)
{
  IPPacketModule = _IPPacketModule;
}

//--------------------------------------------------------------------

void ICMPPacketModule_c::handlePacket(
  char* ICMPPacket, int ICMPPacketLen,
  char* IPHeader, int IPHeaderLen,
  uint8_t type, uint8_t code
)
{

  header = *((struct ICMPHeader_t *)ICMPPacket);
  endianSwap((uint8_t*)&(header.checksum), 2);



  printf("******************************************************\n");
  printf("*****ICMPPacketModule_c::handleCurrentPacket start****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("*****ICMPPacketModule_c::handleCurrentPacket end****\n");
  printf("****************************************************\n");


  if (type == 0x03 && code == 0x00) {
    handleRouterTableFail(IPHeader, IPHeaderLen);
  }
  else {
    printf("???????????TODO: ICMP only support handleRouterTableFail\n");
  }
}

void ICMPPacketModule_c::sendPacket(
  uint8_t type, uint8_t code,
  char* upLayerPacket, int upLayerPacketLen
)
{
  char* packet     = upLayerPacket    - sizeof(struct ICMPHeader_t);
  int packetLen  = upLayerPacketLen + sizeof(struct ICMPHeader_t);

  header.type     = type;
  header.code     = code;
  header.checksum = 0x0000;
  *((struct ICMPHeader_t *)packet) = header;

  header.checksum = checksumBase((uint16_t *)packet, packetLen, 0);
  ((struct ICMPHeader_t *)packet)->checksum = header.checksum;
  endianSwap(((uint8_t*)packet) + 2, 2);




  printf("\n\n");
  printf("******************************************************\n");
  printf("*********ICMPPacketModule_c::sendPacket start*********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("*********ICMPPacketModule_c::sendPacket end*********\n");
  printf("****************************************************\n");

/*
  IPPacketModule->sendPacket(
    TTL_INIT, 0x1, uint32_t daddr, //TODO: should high layer know ip, mac, iface?
    packet, packetLen
  );
*/
}


//--------------------------------------------------------------------

void ICMPPacketModule_c::handleRouterTableFail(char* IPHeader, int IPHeaderLen)
{

}

//--------------------------------------------------------------------

void ICMPPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------ICMP Packet start---------------\n");
  printf("type:     0x%01x\n", header.type);
  printf("code:     0x%01x\n", header.code);
  printf("checksum: 0x%02x\n", header.checksum);
  printf("^^^^^^^^^^^^^^^ICMP Packet end^^^^^^^^^^^^^^^\n");
}
