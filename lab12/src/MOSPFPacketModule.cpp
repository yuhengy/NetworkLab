#include "MOSPFPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "IPPacketModule.h"
#include <stdio.h>
#include <string.h>

#define NEIGHBOUR_BROARDCAST_IP 0xe0000005  // 224.0.0.5
#define MOSPF_TYPE_HELLO 1
#define MOSPF_TYPE_LSU 4

#define MOSPF_HELLOINT    5 // 5 seconds
#define MOSPF_LSUINT    30  // 30 seconds
#define MOSPF_DATABASE_TIMEOUT    40  // 40 seconds

void MOSPFPacketModule_c::addIPAddr(uint32_t IPAddr)
{
  IPList.push_back(IPAddr);
}

void MOSPFPacketModule_c::addIPPacketModule(IPPacketModule_c* _IPPacketModule)
{
  IPPacketModule = _IPPacketModule;
}

//--------------------------------------------------------------------

void MOSPFPacketModule_c::startSubthread()
{
  hello = std::thread(&MOSPFPacketModule_c::sendHelloThread, this);
}

void MOSPFPacketModule_c::sendHelloThread()
{

  struct __attribute__ ((packed)) helloPacket_t {
    uint32_t mask;   // network mask associated with this interface
    uint16_t helloint; // number of seconds between hellos from this router
    uint16_t padding;  // set to zero
  } helloPacket;

  helloPacket.mask = 0xffffff00;
  helloPacket.helloint = MOSPF_HELLOINT;
  helloPacket.padding = 0x0000;

  while (true) {
    char *packet = (char*)malloc(
      MOSPF_HEADER_LEN +
      sizeof(helloPacket)
    );
    packet += MOSPF_HEADER_LEN;

    *((struct helloPacket_t *)packet) = helloPacket;
    endianSwap(((uint8_t*)packet)     , 4);
    endianSwap(((uint8_t*)packet) + 4 , 2);
    endianSwap(((uint8_t*)packet) + 6 , 2);

    sendPacket(MOSPF_TYPE_HELLO, packet, sizeof(helloPacket));
  }
}


//--------------------------------------------------------------------

void MOSPFPacketModule_c::handlePacket(char* MOSPFPacket, int MOSPFPacketLen)
{
  header = *((struct MOSPFHeader_t *)MOSPFPacket);
  endianSwap((uint8_t*)&(header.len)     , 2);
  endianSwap((uint8_t*)&(header.rid)     , 4);
  endianSwap((uint8_t*)&(header.aid)     , 4);
  // checksum always compute from packet
  // and same endian in packet and header
  //endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.padding) , 2);


  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::handleCurrentPacket end*****\n");
  printf("****************************************************\n");


  if (header.version != 2) {
    printf("Error: received mospf packet with incorrect version (%d)\n", header.version);
    return ;
  }
  if (checksumBase((uint16_t *)MOSPFPacket, header.len, 0) != 0) {
    printf("Error: received mospf packet with non-zero checksum (%x)\n",
      checksumBase((uint16_t *)MOSPFPacket, header.len, 0));
    return ;
  }
  if (header.aid != 0) {
    printf("Error: received mospf packet with incorrect area id\n");
    return ;
  }

  switch (header.type) {
    case MOSPF_TYPE_HELLO:
      handleHello();
      break;
    case MOSPF_TYPE_LSU:
      handleLSU();
      break;
    default:
      printf("Error: received mospf packet with unknown type (%d).", header.type);
      break;
  }
}

void MOSPFPacketModule_c::sendPacket(
  uint8_t type, char* upLayerPacket, int upLayerPacketLen
)
{
  char* packet  = upLayerPacket    - MOSPF_HEADER_LEN;
  int packetLen = upLayerPacketLen + MOSPF_HEADER_LEN;

  header.version  = 0x2;
  header.type     = type;
  header.len      = packetLen;
  header.rid      = *(IPList.begin());
  header.aid      = 0x00000000;
  header.checksum = 0x0000;
  header.padding  = 0x0000;


  *((struct MOSPFHeader_t *)packet) = header;
  endianSwap(((uint8_t*)packet) + 2 , 2);
  endianSwap(((uint8_t*)packet) + 4 , 4);
  endianSwap(((uint8_t*)packet) + 8 , 4);
  endianSwap(((uint8_t*)packet) + 14, 2);

  header.checksum = checksumBase((uint16_t *)packet, packetLen, 0);
  ((struct MOSPFHeader_t *)packet)->checksum = header.checksum;



  printf("\n\n");
  printf("******************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket start**********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket end**********\n");
  printf("****************************************************\n");

  for (std::list<uint32_t>::iterator iter =
    IPList.begin(); iter != IPList.end(); iter++){

    char *copiedPacket = (char*)malloc(
      ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + packetLen
    );
    copiedPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN;
    memcpy(copiedPacket, packet, packetLen);

    IPPacketModule->sendPacket(
      TTL_INIT, 0x5a, *iter, NEIGHBOUR_BROARDCAST_IP, 0x5,
      copiedPacket, packetLen
    );
  }
  free(packet);
}


//--------------------------------------------------------------------

void MOSPFPacketModule_c::handleHello()
{
  printf("TODO: MOSPFPacketModule_c::handleHello.\n");
}

void MOSPFPacketModule_c::handleLSU()
{
  printf("TODO: MOSPFPacketModule_c::handleLSU.\n");
}

//--------------------------------------------------------------------

void MOSPFPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------MOSPF Packet Header start---------------\n");
  printf("version:  0x%01x\n", header.version);
  printf("type:     0x%01x\n", header.type);
  printf("len:      0x%02x\n", header.len);
  printf("rid:      0x%04x\n", header.rid);
  printf("aid:      0x%04x\n", header.aid);
  printf("checksum: 0x%02x\n", header.checksum);
  printf("padding:  0x%02x\n", header.padding);
  printf("^^^^^^^^^^^^^^^MOSPF Packet Header end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printIPList()
{
  printf("---------------MOSPF IPList start---------------\n");
  for (std::list<uint32_t>::iterator iter =
    IPList.begin(); iter != IPList.end(); iter++){

    printf("IPAddr: 0x%08x\n", *iter);
  }
  printf("^^^^^^^^^^^^^^^MOSPF IPList end^^^^^^^^^^^^^^^\n");
}

//--------------------------------------------------------------------

MOSPFPacketModule_c::~MOSPFPacketModule_c()
{
  hello.join();
}
