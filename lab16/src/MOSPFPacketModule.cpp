#include "MOSPFPacketModule.h"

#include "common.h"
#include "endianSwap.h"
#include "checksumBase.h"
#include "dijkstra.h"
#include "IPPacketModule.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>

#define NEIGHBOUR_BROARDCAST_IP 0xe0000005  // 224.0.0.5
#define MOSPF_TYPE_HELLO 1
#define MOSPF_TYPE_LSU 4

#define MOSPF_HELLOINT    5 // 5 seconds
#define MOSPF_NEIGHBOUR_TIMEOUT    15 // 15 seconds
#define MOSPF_LSUINT    30  // 30 seconds
#define MOSPF_DATABASE_TIMEOUT    40  // 40 seconds
#define LSU_TTL_INIT 64

void MOSPFPacketModule_c::addIPAddr(uint32_t IPAddr)
{
  IPList.push_back(IPAddr);
}

void MOSPFPacketModule_c::addRouterTable(routerTable_c* _routerTable)
{
  routerTable = _routerTable;
}

void MOSPFPacketModule_c::addIPPacketModule(IPPacketModule_c* _IPPacketModule)
{
  IPPacketModule = _IPPacketModule;
}

//--------------------------------------------------------------------

void MOSPFPacketModule_c::startSubthread()
{
  hello = std::thread(&MOSPFPacketModule_c::sendHelloThread, this);
  LSU = std::thread(&MOSPFPacketModule_c::sendLSUThread, this);
  neighbourTimeout = std::thread(&MOSPFPacketModule_c::neighbourTimeoutThread, this);
  nodeTimeout = std::thread(&MOSPFPacketModule_c::nodeTimeoutThread, this);
}

void MOSPFPacketModule_c::sendHelloThread()
{
  struct helloContent_t content;  // in machine endian
  content.mask = 0xffffff00;
  content.helloint = MOSPF_HELLOINT;
  content.padding = 0x0000;


#if 0
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
      sizeof(struct helloContent_t)
    );
    packet += MOSPF_HEADER_LEN;

    *((struct helloContent_t *)packet) = content;
    endianSwap(((uint8_t*)packet)     , 4);
    endianSwap(((uint8_t*)packet) + 4 , 2);
    endianSwap(((uint8_t*)packet) + 6 , 2);

    sendPacket(MOSPF_TYPE_HELLO, packet, sizeof(struct helloContent_t), 0, 0);
  }
}

void MOSPFPacketModule_c::sendLSUThread()
{
  //sleep(1);  // avoid send at the same time with hello
  while (true) {
    for (int slept = 0; slept < MOSPF_LSUINT; slept++) {
      if (neighbourInfoMap_changed) {
        neighbourInfoMap_changed = false;
        break;
      }
      sleep(1);
    }

    // create from neighbourInfoMap
    neighbourInfoMap_mutex.lock();

    struct LSU1Content_t content1;
    content1.seq = LSUSequence++;
    content1.ttl = LSU_TTL_INIT;
    content1.unused = 0x00;
    content1.nadv = neighbourInfoMap.size();


    std::list<uint32_t> nonReceivedNet;
    for (std::list<uint32_t>::iterator iter =
      IPList.begin(); iter != IPList.end(); iter++){
        nonReceivedNet.push_back((*iter) & 0xffffff00);
    }
    std::list<struct LSU2Content_t> content2;
    for (std::map<uint32_t, struct neighbourInfo_t>::iterator iter =
      neighbourInfoMap.begin(); iter != neighbourInfoMap.end(); iter++){
        content2.push_back({
          iter->second.nbr_ip & iter->second.nbr_mask,
          iter->second.nbr_mask,
          iter->second.nbr_id
        });
        nonReceivedNet.remove(iter->second.nbr_ip & iter->second.nbr_mask);
    }

    neighbourInfoMap_mutex.unlock();

    for (std::list<uint32_t>::iterator iter =
      nonReceivedNet.begin(); iter != nonReceivedNet.end(); iter++){
        content2.push_back({*iter, 0xffffff00, 0x00000000});
        (content1.nadv)++;
    }

  #if 0
    printf("******************************************************\n");
    printf("******MOSPFPacketModule_c::sendLSUThread start*****\n");
    printf("******************************************************\n");
    debug_printLSUContent(content1, content2);
    printf("****************************************************\n");
    printf("******MOSPFPacketModule_c::sendLSUThread end*****\n");
    printf("****************************************************\n");
  #endif

    // change endian
    char *packet = (char*)malloc(
      MOSPF_HEADER_LEN +
      sizeof(struct LSU1Content_t) +
      sizeof(struct LSU2Content_t) * content1.nadv
    );
    packet += MOSPF_HEADER_LEN;

    *((struct LSU1Content_t *)packet) = content1;
    endianSwap(((uint8_t*)packet)    , 2);
    endianSwap(((uint8_t*)packet) + 4, 4);

    char* tempPointer = packet + sizeof(struct LSU1Content_t);
    for (std::list<struct LSU2Content_t>::iterator iter =
      content2.begin(); iter != content2.end(); iter++){

      *((struct LSU2Content_t *)tempPointer) = *iter;
      endianSwap(((uint8_t*)tempPointer)    , 4);
      endianSwap(((uint8_t*)tempPointer) + 4, 4);
      endianSwap(((uint8_t*)tempPointer) + 8, 4);
      tempPointer += sizeof(struct LSU2Content_t);
    }

    // send
    sendPacket(
      MOSPF_TYPE_LSU, packet,
      sizeof(struct LSU1Content_t) + sizeof(struct LSU2Content_t) * content1.nadv,
      0, 0
    );
  }
}

void MOSPFPacketModule_c::neighbourTimeoutThread()
{
  while (true) {
    sleep(1);

    neighbourInfoMap_mutex.lock();

    std::list<uint32_t> timeoutRidList;
    for (std::map<uint32_t, struct neighbourInfo_t>::iterator iter =
      neighbourInfoMap.begin(); iter != neighbourInfoMap.end(); iter++){

      (iter->second.alive)++;
      if (iter->second.alive == MOSPF_NEIGHBOUR_TIMEOUT) {
        timeoutRidList.push_back(iter->first);
      }
    }
    for (std::list<uint32_t>::iterator iter =
      timeoutRidList.begin(); iter != timeoutRidList.end(); iter++){
        neighbourInfoMap.erase(*iter);
        neighbourInfoMap_changed = true;
    }

    neighbourInfoMap_mutex.unlock();
  }
}

void MOSPFPacketModule_c::nodeTimeoutThread()
{
  while (true) {
    sleep(1);

    nodeInfoMap_mutex.lock();

    std::list<uint32_t> timeoutRidList;
    for (std::map<uint32_t, struct nodeInfo_t>::iterator iter =
      nodeInfoMap.begin(); iter != nodeInfoMap.end(); iter++){

      (iter->second.alive)++;
      if (iter->second.alive == MOSPF_DATABASE_TIMEOUT) {
        timeoutRidList.push_back(iter->first);
      }
    }
    bool nodeInfoMap_changed = false;
    for (std::list<uint32_t>::iterator iter =
      timeoutRidList.begin(); iter != timeoutRidList.end(); iter++){
        nodeInfoMap.erase(*iter);
        nodeInfoMap_changed = true;
    }
    nodeInfoMap_mutex.unlock();

    if (nodeInfoMap_changed) updateRouterTable();

  }
}


//--------------------------------------------------------------------

void MOSPFPacketModule_c::handlePacket(
  char* MOSPFPacket, int MOSPFPacketLen, uint32_t srcIP, uint32_t ifaceIP
)
{
  struct MOSPFHeader_t header = *((struct MOSPFHeader_t *)MOSPFPacket);
  endianSwap((uint8_t*)&(header.len)     , 2);
  endianSwap((uint8_t*)&(header.rid)     , 4);
  endianSwap((uint8_t*)&(header.aid)     , 4);
  // checksum always compute from packet
  // and same endian in packet and header
  //endianSwap((uint8_t*)&(header.checksum), 2);
  endianSwap((uint8_t*)&(header.padding) , 2);


#if 0
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::handleCurrentPacket start*****\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader(header);
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
      handleLSU(
        MOSPFPacket + MOSPF_HEADER_LEN, MOSPFPacketLen- MOSPF_HEADER_LEN,
        header.rid, ifaceIP);
      break;
    default:
      printf("Error: received mospf packet with unknown type (%d).", header.type);
      break;
  }
}

void MOSPFPacketModule_c::sendPacket(
  uint8_t type, char* upLayerPacket, int upLayerPacketLen,
  uint32_t rid, uint32_t exclueIfaceIP
)
{
  char* packet  = upLayerPacket    - MOSPF_HEADER_LEN;
  int packetLen = upLayerPacketLen + MOSPF_HEADER_LEN;

  struct MOSPFHeader_t header;
  header.version  = 0x2;
  header.type     = type;
  header.len      = packetLen;
  if (rid == 0) {
    header.rid    = *(IPList.begin());
  }
  else {
    header.rid    = rid;
  }
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

#if 0
  printf("\n\n");
  printf("******************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket start**********\n");
  printf("******************************************************\n");
  debug_printCurrentPacketHeader(header);
  printf("****************************************************\n");
  printf("**********MOSPFPacketModule_c::sendPacket end**********\n");
  printf("****************************************************\n");
#endif

  for (std::list<uint32_t>::iterator iter =
    IPList.begin(); iter != IPList.end(); iter++){

    if (*iter != exclueIfaceIP) {
      char *copiedPacket = (char*)malloc(
        ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN + packetLen
      );
      copiedPacket += ETHER_HEADER_LEN + DEFAULTIP_HEADER_LEN;
      memcpy(copiedPacket, packet, packetLen);

      IPServe_mutex.lock();
      // TODO: LSU should have different destIP
      IPPacketModule->sendPacket(
        TTL_INIT, 0x5a, *iter, NEIGHBOUR_BROARDCAST_IP, 0x5,
        copiedPacket, packetLen
      );
      IPServe_mutex.unlock();
    }
  }
  //free(packet);  // TODO: free when forward
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

#if 0
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
  std::map<uint32_t, struct neighbourInfo_t>::iterator iter =
    neighbourInfoMap.find(rid);
  if (iter == neighbourInfoMap.end()) {
    neighbourInfoMap_changed = true;
  }
  neighbourInfoMap[rid] = {rid, srcIP, content.mask, 0};

#if 0
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::neighbourInfoMap start*****\n");
  printf("******************************************************\n");
  debug_printNeighbourInfoMap();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::neighbourInfoMap end*****\n");
  printf("****************************************************\n");
#endif

  neighbourInfoMap_mutex.unlock();

  
}

void MOSPFPacketModule_c::handleLSU(
  char* LSUPacket, int LSUPacketLen, uint32_t rid, uint32_t ifaceIP
)
{
  // Do not include itself
  if (rid == *(IPList.begin())) return;

  // Save the packet info
  struct LSU1Content_t content1 = *((struct LSU1Content_t *)LSUPacket);
  endianSwap((uint8_t*)&(content1.seq) , 2);
  endianSwap((uint8_t*)&(content1.nadv), 4);

  char* tempPointer = LSUPacket + sizeof(struct LSU1Content_t);
  std::list<struct LSU2Content_t> content2;
  for (int index = 0; index < content1.nadv; index++){
    struct LSU2Content_t content2Entry = *((struct LSU2Content_t *)tempPointer);
    endianSwap((uint8_t*)&(content2Entry.network), 4);
    endianSwap((uint8_t*)&(content2Entry.mask)   , 4);
    endianSwap((uint8_t*)&(content2Entry.rid)    , 4);
    content2.push_back(content2Entry);
    tempPointer += sizeof(struct LSU2Content_t);
  }
  #if 0
    printf("******************************************************\n");
    printf("******MOSPFPacketModule_c::handleLSU start*****\n");
    printf("******************************************************\n");
    debug_printLSUContent(content1, content2);
    printf("****************************************************\n");
    printf("******MOSPFPacketModule_c::handleLSU end*****\n");
    printf("****************************************************\n");
  #endif

  // Update the database
  IPServe_mutex.lock();

  std::map<uint32_t, struct nodeInfo_t>::iterator iter =
    nodeInfoMap.find(rid);
  if (iter == nodeInfoMap.end()) {
    nodeInfoMap[rid] = {
      rid, content1.seq, content1.nadv, 0, content2
    };
  }
  else if (iter->second.seq < content1.seq) {
    nodeInfoMap[rid] = {
      rid, content1.seq, content1.nadv, 0, content2
    };
  }
  else {
    //printf("TODO: avoid receive same LSU for another time\n");
  }

#if 1
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::nodeInfoMap start*****\n");
  printf("******************************************************\n");
  debug_printNodeInfoMap();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::nodeInfoMap end*****\n");
  printf("****************************************************\n");
#endif

  updateRouterTable();

  IPServe_mutex.unlock();


  // Forward the packet
  // TODO: avoid send to the same iface
  if (content1.ttl > 0) {
    (((struct LSU1Content_t *)LSUPacket)->ttl)--;
    sendPacket(MOSPF_TYPE_LSU, LSUPacket, LSUPacketLen, rid, ifaceIP);
  }
}

//--------------------------------------------------------------------

void MOSPFPacketModule_c::updateRouterTable()
{
  nodeInfoMap_mutex.lock();

  // total node number
  int numNode = nodeInfoMap.size() + 1;

  // init data
  int graph[numNode * numNode];
  int nextIndex[numNode];
  for (int i = 0; i < numNode; i++) {
    for (int j = 0; j < numNode; j++) {
      graph[i * numNode + j] = -1;
    }
  }

  // create a ridList
  std::vector<uint32_t> ridList;
  ridList.push_back(*(IPList.begin()));
  for (std::map<uint32_t, struct nodeInfo_t>::iterator iter =
    nodeInfoMap.begin(); iter != nodeInfoMap.end(); iter++){
      ridList.push_back(iter->first);
  }

  // compute the graph
  for (std::map<uint32_t, struct nodeInfo_t>::iterator iter =
    nodeInfoMap.begin(); iter != nodeInfoMap.end(); iter++){

    // find index of one end of the link
    std::vector<uint32_t>::iterator result = 
      find(ridList.begin(), ridList.end(), iter->first);
    int index = distance(ridList.begin(), result);
    for (std::list<struct LSU2Content_t>::iterator iter2 =
      iter->second.content2.begin(); iter2 != iter->second.content2.end(); iter2++){
      
      // find index of one end of the link
      std::vector<uint32_t>::iterator result2 = 
        find(ridList.begin(), ridList.end(), iter2->rid);
      int index2 = distance(ridList.begin(), result2);

      // add link
      graph[index * numNode + index2] = 1;
      graph[index2 * numNode + index] = 1;
    }
  }

  // get dijkstra result (key:targetRid, value:nextRid)
  std::map<uint32_t, uint32_t> nextRid;
  dijkstra(graph, numNode, nextIndex);
  for (int index = 1; index < numNode; index++) {
    nextRid[ridList[index]] = ridList[nextIndex[index]];
  }

  // add entry to router table
  // TODO: give routerTable a new mutex
  routerTable->clearMOSPFEntry();
  for (std::map<uint32_t, uint32_t>::iterator iter =
    nextRid.begin(); iter != nextRid.end(); iter++){

    for (std::list<struct LSU2Content_t>::iterator iter2 =
      nodeInfoMap[iter->first].content2.begin();
      iter2 != nodeInfoMap[iter->first].content2.end(); iter2++){

      routerTable->addMOSPFEntry(iter2->network, iter2->mask, neighbourInfoMap[iter->second].nbr_ip);
    }
  }


#if 1
  printf("******************************************************\n");
  printf("******MOSPFPacketModule_c::updateRouterTable start*****\n");
  printf("******************************************************\n");
  routerTable->debug_printRouterTable();
  printf("****************************************************\n");
  printf("******MOSPFPacketModule_c::updateRouterTable end*****\n");
  printf("****************************************************\n");
#endif

  nodeInfoMap_mutex.unlock();
}


//--------------------------------------------------------------------

void MOSPFPacketModule_c::debug_printCurrentPacketHeader(struct MOSPFHeader_t header)
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

void MOSPFPacketModule_c::debug_printIPList()
{
  printf("---------------MOSPF IPList start---------------\n");
  for (std::list<uint32_t>::iterator iter =
    IPList.begin(); iter != IPList.end(); iter++){

    printf("IPAddr: 0x%08x\n", *iter);
  }
  printf("^^^^^^^^^^^^^^^MOSPF IPList end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printHelloContent(struct helloContent_t content)
{
  printf("---------------MOSPF Hello Content start---------------\n");
  printf("mask:     0x%08x\n", content.mask);
  printf("helloint: 0x%04x\n", content.helloint);
  printf("padding:  0x%04x\n", content.padding);
  printf("^^^^^^^^^^^^^^^MOSPF Hello Content end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printNeighbourInfoMap()
{
  printf("---------------MOSPF neighbourInfoMap start---------------\n");
  for (std::map<uint32_t, struct neighbourInfo_t>::iterator iter =
    neighbourInfoMap.begin(); iter != neighbourInfoMap.end(); iter++){

    printf("nbr_id: 0x%08x;  ", iter->second.nbr_id);
    printf("nbr_ip: 0x%08x;  ", iter->second.nbr_ip);
    printf("nbr_mask: 0x%08x;  ", iter->second.nbr_mask);
    printf("alive: 0x%02x\n", iter->second.alive);
  }
  printf("^^^^^^^^^^^^^^^MOSPF neighbourInfoMap end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printNodeInfoMap()
{
  printf("---------------MOSPF nodeInfoMap start---------------\n");
  for (std::map<uint32_t, struct nodeInfo_t>::iterator iter =
    nodeInfoMap.begin(); iter != nodeInfoMap.end(); iter++){

    printf("----->");
    printf("rid: 0x%08x;  ", iter->second.rid);
    printf("seq: 0x%08x;  ", iter->second.seq);
    printf("nadv: 0x%08x;  ", iter->second.nadv);
    printf("alive: 0x%02x\n", iter->second.alive);
    for (std::list<struct LSU2Content_t>::iterator iter2 =
      iter->second.content2.begin();
      iter2 != iter->second.content2.end(); iter2++){

      printf("network: 0x%08x;  ", iter2->network);
      printf("mask: 0x%04x;  ", iter2->mask);
      printf("rid: 0x%04x\n", iter2->rid);
    }
  }
  printf("^^^^^^^^^^^^^^^MOSPF nodeInfoMap end^^^^^^^^^^^^^^^\n");
}

void MOSPFPacketModule_c::debug_printLSUContent(
  struct LSU1Content_t content1, std::list<struct LSU2Content_t> content2
)
{
  printf("---------------MOSPF LSU Content start---------------\n");
  printf("seq:    0x%08x\n", content1.seq);
  printf("ttl:    0x%04x\n", content1.ttl);
  printf("unused: 0x%04x\n", content1.unused);
  printf("nadv:   0x%04x\n", content1.nadv);
  for (std::list<struct LSU2Content_t>::iterator iter =
    content2.begin(); iter != content2.end(); iter++){

    printf("network: 0x%08x;  ", iter->network);
    printf("mask: 0x%04x;  ", iter->mask);
    printf("rid: 0x%08x\n", iter->rid);
  }
  printf("^^^^^^^^^^^^^^^MOSPF LSU Content end^^^^^^^^^^^^^^^\n");
}

//--------------------------------------------------------------------

MOSPFPacketModule_c::~MOSPFPacketModule_c()
{
  hello.join();
  LSU.join();
  neighbourTimeout.join();
  nodeTimeout.join();
}
