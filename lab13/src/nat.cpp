#include "nat.h"

#include "TCPPacketModule.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

nat_c::nat_c(const char* fileName)
{
   FILE* fp = fopen(fileName , "r");
   if(fp == NULL) {
      printf("Error: cannot open net config file.\n");
   }
  char line[100];
  while ( fgets(line, 100, fp) != NULL ) {
    if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
    if (memcmp(line, "internal-iface: ", strlen("internal-iface: ")) == 0) {
      strcpy(intIfaceName, line + strlen("internal-iface: ") );  // End with \n\0
    }
    else if (memcmp(line, "external-iface: ", strlen("external-iface: ")) == 0) {
      strcpy(extIfaceName, line + strlen("external-iface: ") );
    }
    else if (memcmp(line, "dnat-rules: ", strlen("dnat-rules: ")) == 0) {
      char* lineCurse = line + strlen("dnat-rules: ");
      struct rule_t rule;

      readIPTCP(&lineCurse, &(rule.external_ip), &(rule.external_port));
      lineCurse += (strlen(" -> "));
      readIPTCP(&lineCurse, &(rule.internal_ip), &(rule.internal_port));

      ruleList.push_back(rule);
    }
  }
  fclose(fp);
}


void nat_c::addTCPPacketModule(TCPPacketModule_c* _TCPPacketModule)
{
  if (strcmp(intIfaceName, _TCPPacketModule->iface->getName()) == 0) {
    intTCPPacketModule = _TCPPacketModule;
    printf("**********************************************\n");
    printf("**********init nat intTCPPacketModule*********\n");
    printf("**********************************************\n");
  }
  else if (strcmp(extIfaceName, _TCPPacketModule->iface->getName()) == 0) {
    extTCPPacketModule = _TCPPacketModule;
    printf("**********************************************\n");
    printf("**********init nat extTCPPacketModule*********\n");
    printf("**********************************************\n");
  }
  else {
    printf("Error: Iface is not define as int or ext Iface.\n");
  }
  
}

//--------------------------------------------------------------------

void nat_c::translate(
  TCPPacketModule_c* _TCPPacketModule, char* TCPContent, int TCPContentLen,
  uint32_t sIP, uint16_t sPort, uint32_t dIP, uint16_t dPort
)
{
  if (_TCPPacketModule == intTCPPacketModule) {

    // exist
    auto iter1 = netTable.find(std::pair<uint32_t, uint16_t>(dIP, dPort));
    if (iter1 != netTable.end()) {
      for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
        if (iter2->internal_ip == sIP && iter2->internal_port == sPort) {
          extTCPPacketModule->sendPacket(
            TCPContent, TCPContentLen,
            iter2->external_ip, iter2->external_port, dIP, dPort
          );
          return;
        }
      }
    }
    else {
      netTable[std::pair<uint32_t, uint16_t>(dIP, dPort)] = std::list<struct natMap_t>();
    }

    // create a new
    uint32_t newIP = extTCPPacketModule->iface->getIP();
    uint16_t newPort = assignExternalPort();
    auto iter = netTable.find(std::pair<uint32_t, uint16_t>(dIP, dPort));
    if (iter != netTable.end()) {
      iter->second.push_back({
        dIP, dPort, sIP, sPort, newIP, newPort
      });
    }
    else {
      printf("Error: insert a entry to netTable but cannnot find it.\n");
    }
    extTCPPacketModule->sendPacket(
      TCPContent, TCPContentLen, newIP, newPort, dIP, dPort
    );
  }

  else if (_TCPPacketModule == extTCPPacketModule) {
    // exist
    auto iter1 = netTable.find(std::pair<uint32_t, uint16_t>(sIP, sPort));
    if (iter1 != netTable.end()) {
      for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
        if (iter2->external_ip == dIP && iter2->external_port == dPort) {
          intTCPPacketModule->sendPacket(
            TCPContent, TCPContentLen,
            sIP, sPort, iter2->internal_ip, iter2->internal_port
          );
          return;
        }
      }
    }
    else {
      netTable[std::pair<uint32_t, uint16_t>(sIP, sPort)] = std::list<struct natMap_t>();
    }

    // create a new
    uint32_t newIP = 0;
    uint16_t newPort = 0;
    printf("------>Try to find dIP 0x%x; dPort %d.\n", dIP, dPort);
    for (auto iter = ruleList.begin(); iter != ruleList.end(); iter++) {
      if (iter->external_ip == dIP && iter->external_port == dPort) {
        printf("------------>Find Port %d\n", newPort);
        newIP = iter->internal_ip;
        newPort = iter->internal_port;
      }
    }
    if (newIP == 0 && newPort == 0) {
      printf("Error: cannot find the port in Nat ruleList.\n");
    }
    auto iter = netTable.find(std::pair<uint32_t, uint16_t>(sIP, sPort));
    if (iter != netTable.end()) {
      iter->second.push_back({
        sIP, sPort, newIP, newPort, dIP, dPort
      });
    }
    else {
      printf("Error: insert a entry to netTable but cannnot find it.\n");
    }
    intTCPPacketModule->sendPacket(
      TCPContent, TCPContentLen, sIP, sPort, newIP, newPort
    );
  }

  else {
    printf("Error: Nat does not recognize a TCPPacketModule.\n");
  }

}

//--------------------------------------------------------------------

void nat_c::readIPTCP(char** lineCurse, uint32_t* IP, uint16_t* port)
{
  *IP = 0;
  uint32_t byte = 0;
  for (int i = 0; i < 4; i++) {
    while (**lineCurse != '.' && **lineCurse != ':') {
      byte *= 10;
      byte += (**lineCurse - '0');
      (*lineCurse)++;
    }
    *IP <<= 8;
    *IP += byte;
    byte = 0;
    (*lineCurse)++;
  }

  *port = 0;
  while (**lineCurse != ' ' && **lineCurse != '\0' && **lineCurse != '\n') {
    *port *= 10;
    *port += (**lineCurse - '0');
    (*lineCurse)++;
  }
}


uint16_t nat_c::assignExternalPort()
{
  static uint16_t port = 1;
  assert(port != 0xffff && "Nat cannot have so many Port ID for now.");
  return port++;
}


void nat_c::debug_printRuleList()
{
  printf("---------------Nat RuleLis start---------------\n");
  printf("intIfaceName: %s; extIfaceName: %s\n", intIfaceName, extIfaceName);
  for (auto iter = ruleList.begin(); iter != ruleList.end(); iter++){
    printf("(external_ip: 0x%08x, external_port: 0x%04x)",
      iter->external_ip, iter->external_port);
    printf(" --> ");
    printf("(internal_ip: 0x%08x, internal_port: 0x%04x)\n",
      iter->internal_ip, iter->internal_port);
  }
  printf("^^^^^^^^^^^^^^^Nat RuleLis end^^^^^^^^^^^^^^^\n");
}



