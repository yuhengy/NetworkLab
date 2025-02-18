#ifndef __NAT_H__
#define __NAT_H__


class TCPPacketModule_c;
#include <unistd.h>
#include <list>
#include <map>
#include <thread>
#include <mutex>


class nat_c {
public:
  nat_c(const char* fileName);
  void addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule);
  void startSubthread();

  bool translate(
    TCPPacketModule_c* _TCPPacketModule, char* TCPContent, int TCPContentLen,
    uint32_t sIP, uint16_t sPort, uint32_t dIP, uint16_t dPort
  );

  void debug_printRuleList();

  ~nat_c();


private:
  char intIfaceName[20];
  char extIfaceName[20];
  TCPPacketModule_c* intTCPPacketModule;
  TCPPacketModule_c* extTCPPacketModule;

  // sub thread
  void tableTimeoutThread();
  std::thread tableTimeout;


  struct rule_t {
    uint32_t external_ip;    // ip address seen in public network (the ip address of external interface)
    uint16_t external_port;    // port seen in public network
    uint32_t internal_ip;    // ip address seen in private network
    uint16_t internal_port;    // port seen in private network
  };
  std::list<struct rule_t> ruleList;
  void readIPTCP(char** lineCurse, uint32_t* IP, uint16_t* port);


  struct natMap_t {
    uint32_t remote_ip;      // ip address of the real peer
    uint16_t remote_port;    // port of the real peer
    uint32_t internal_ip;    // ip address seen in private network
    uint16_t internal_port;    // port seen in private network
    uint32_t external_ip;    // ip address seen in public network (the ip address of external interface)
    uint16_t external_port;    // port seen in public network (assigned by nat)

    uint8_t  alive;
  };
  std::map<std::pair<uint32_t, uint16_t>,
           std::list<struct natMap_t>> netTable;
  std::mutex netTable_mutex;
  uint16_t assignExternalPort();

};









#endif
