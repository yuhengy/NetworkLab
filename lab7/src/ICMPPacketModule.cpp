#include "ICMPPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "IPPacketModule.h"
#include <stdio.h>
#include <string.h>
#include <algorithm>

void ICMPPacketModule_c::addIPPacketModule(IPPacketModule_c* _IPPacketModule)
{
  IPPacketModule = _IPPacketModule;
}

//--------------------------------------------------------------------

void ICMPPacketModule_c::handlePacket(
  char* ICMPPacket, int ICMPPacketLen, uint32_t soureIP,
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
    handleRouterTableFail(IPHeader, IPHeaderLen, soureIP);
  }
  else {
    printf("???????????TODO: ICMP only support handleRouterTableFail\n");
  }
}

void ICMPPacketModule_c::sendPacket(
  uint8_t type, uint8_t code,
  char* upLayerPacket, int upLayerPacketLen, uint32_t targetIP
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




  printf("\n\n");
  printf("******************************************************\n");
  printf("*********ICMPPacketModule_c::sendPacket start*********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("*********ICMPPacketModule_c::sendPacket end*********\n");
  printf("****************************************************\n");


  IPPacketModule->sendPacket(
    TTL_INIT, 0x1, 0x00000000, targetIP, 0x5,
    packet, packetLen
  );

}


//--------------------------------------------------------------------

void ICMPPacketModule_c::handleRouterTableFail(char* IPHeader, int IPHeaderLen, uint32_t soureIP)
{

  char* packet = (char*)malloc(ETHER_HEADER_LEN + 20 + 4 + 4 + IPHeaderLen + 8);
  char* packetContent = packet + ETHER_HEADER_LEN + 20 + 4;
  memset(packetContent, 0, 4);
  memcpy(packetContent + 4, IPHeader, IPHeaderLen);
  memcpy(packetContent + 4 + IPHeaderLen, IPHeader + IPHeaderLen, 8);

  sendPacket(
    0x03, 0x00,
    packetContent, 4 + IPHeaderLen + 8, soureIP
  );
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
