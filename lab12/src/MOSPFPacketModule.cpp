#include "MOSPFPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "IPPacketModule.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PRINT_STATUS
//#define PRINT_PACKET_RECEIVE
//#define PRINT_PACKET_SEND

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
  struct helloContent_t content;  // in machine endian
  content.mask = 0xffffff00;
  content.helloint = MOSPF_HELLOINT;
  content.padding = 0x0000;


#ifdef PRINT_PACKET_SEND
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::sendHelloThread start*****\n");
  printf("******************************************************\n");
  debug_printHelloContent(content);
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::sendHelloThread end*****\n");
  printf("****************************************************\n");
#endif


  while (true) {
    sleep(MOSPF_HELLOINT);
    
    char *packet = (char*)malloc(  // in net endian
      MOSPF_HEADER_LEN +
      sizeof(content)
    );
    packet += MOSPF_HEADER_LEN;

    *((struct helloContent_t *)packet) = content;
    endianSwap(((uint8_t*)packet)     , 4);
    endianSwap(((uint8_t*)packet) + 4 , 2);
    endianSwap(((uint8_t*)packet) + 6 , 2);

    sendPacket(MOSPF_TYPE_HELLO, packet, sizeof(content));
  }
}


//--------------------------------------------------------------------

void MOSPFPacketModule_c::handlePacket(char* MOSPFPacket, int MOSPFPacketLen, uint32_t srcIP)
{
  header = *((struct MOSPFHeader_t *)MOSPFPacket);
  endianSwap((uint8_t*)&(header.len)     , 2);
  endianSwap((uint8_t*)&(header.rid)     , 4);
  endianSwap((uint8_t*)&(header.aid)     , 4);
  // checksum always compute from packet
  // and same endian in packet and header
  //endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.padding) , 2);


#ifdef PRINT_PACKET_RECEIVE
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::handleCurrentPacket end*****\n");
  printf("****************************************************\n");
#endif


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
      handleHello(
        MOSPFPacket + MOSPF_HEADER_LEN, MOSPFPacketLen- MOSPF_HEADER_LEN,
        header.rid, srcIP
      );
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

#ifdef PRINT_PACKET_SEND
  printf("\n\n");
  printf("******************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket start**********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader();
  printf("****************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket end**********\n");
  printf("****************************************************\n");
#endif

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

void MOSPFPacketModule_c::handleHello(
  char* helloPacket, int helloPacketLen, uint32_t rid, uint32_t srcIP)
{
  struct helloContent_t content;  // in machine endian
  content = *((struct helloContent_t *)helloPacket);
  endianSwap((uint8_t*)&(content.mask)    , 4);
  endianSwap((uint8_t*)&(content.helloint), 2);
  endianSwap((uint8_t*)&(content.padding) , 2);

#ifdef PRINT_PACKET_RECEIVE
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::handleHello start*****\n");
  printf("******************************************************\n");
  debug_printHelloContent(content);
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::handleHello end*****\n");
  printf("****************************************************\n");
#endif

  // update old or insert new
  neighbourInfoMap_mutex.lock();
  neighbourInfoMap[rid] = {rid, srcIP, content.mask, 0};
  neighbourInfoMap_mutex.unlock();

#ifdef PRINT_STATUS
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::neighbourInfoMap start*****\n");
  printf("******************************************************\n");
  debug_printneighbourInfoMap();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::neighbourInfoMap end*****\n");
  printf("****************************************************\n");
#endif
  
}

void MOSPFPacketModule_c::handleLSU()
{
  printf("TODO: MOSPFPacketModule_c::handleLSU.\n");
}

//--------------------------------------------------------------------

void MOSPFPacketModule_c::debug_printCurrentPacketHeader()
{
  printf("---------------MOSPF Packet Header start---------------\n");
  printf("version:  0x%02x\n", header.version);
  printf("type:     0x%02x\n", header.type);
  printf("len:      0x%04x\n", header.len);
  printf("rid:      0x%08x\n", header.rid);
  printf("aid:      0x%08x\n", header.aid);
  printf("checksum: 0x%04x\n", header.checksum);
  printf("padding:  0x%04x\n", header.padding);
  printf("^^^^^^^^^^^^^^^MOSPF Packet Header end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printHelloContent(struct helloContent_t content)
{
  printf("---------------MOSPF Hello Content start---------------\n");
  printf("mask:     0x%08x\n", content.mask);
  printf("helloint: 0x%04x\n", content.helloint);
  printf("padding:  0x%04x\n", content.padding);
  printf("^^^^^^^^^^^^^^^MOSPF Hello Content end^^^^^^^^^^^^^^^\n");
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

void MOSPFPacketModule_c::debug_printneighbourInfoMap()
{
  printf("---------------MOSPF neighbourInfoMap start---------------\n");
  for (std::map<uint32_t, struct neighbourInfo_t>::iterator iter =
    neighbourInfoMap.begin(); iter != neighbourInfoMap.end(); iter++){

    printf("------>");
    printf("nbr_id:   0x%08x\n", iter->second.nbr_id);
    printf("nbr_ip:   0x%08x\n", iter->second.nbr_ip);
    printf("nbr_mask: 0x%08x\n", iter->second.nbr_mask);
    printf("alive:    0x%02x\n", iter->second.alive);
  }
  printf("^^^^^^^^^^^^^^^MOSPF neighbourInfoMap end^^^^^^^^^^^^^^^\n");
}

//--------------------------------------------------------------------

MOSPFPacketModule_c::~MOSPFPacketModule_c()
{
  hello.join();
}
