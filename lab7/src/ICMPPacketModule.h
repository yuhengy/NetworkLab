#ifndef __ICMPPACKETMODULE_H__
#define __ICMPPACKETMODULE_H__

class IPPacketModule_c;
#include <stdint.h>

class ICMPPacketModule_c {
public:
  void addIPPacketModule(IPPacketModule_c* _IPPacketModule);

  void handlePacket(
    char* ICMPPacket, int ICMPPacketLen, uint32_t soureIP,
    char* IPHeader, int IPHeaderLen,
    uint8_t type, uint8_t code
  );
  void sendPacket(
    uint8_t type, uint8_t code,
    char* upLayerPacket, int upLayerPacketLen, uint32_t targetIP
  );

  void debug_printCurrentPacketHeader();


private:
  // configuration
  IPPacketModule_c* IPPacketModule;
  
  // information from IP
  // these information
  // will no be used to call IPPacketModule directly
  char *IPHeaderfromIP;
  int IPHeaderLenfromIP;
  uint8_t typefromIP;
  uint8_t codefromIP;

  // header
  struct __attribute__ ((packed)) ICMPHeader_t {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
  } header;

  // handle packet in this layer
  void handleRouterTableFail(char* IPHeader, int IPHeaderLen, uint32_t soureIP);
  void handleTTLZeroFail(char* IPHeader, int IPHeaderLen, uint32_t soureIP);
  void handleARPFail(char* IPHeader, int IPHeaderLen, uint32_t soureIP);
  void handlePingResp(char* ICMPPacket, int ICMPPacketLen, uint32_t soureIP);


};



#endif
