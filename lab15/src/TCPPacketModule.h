#ifndef __TCPMODULE_H__
#define __TCPMODULE_H__


class IPPacketModule_c;
class TCPSock_c;
#include "iface.h"
#include "nat.h"
#include <stdint.h>
#include <endian.h>

class TCPPacketModule_c {
public:
  iface_c* iface;
  TCPPacketModule_c(iface_c* _iface) { iface = _iface; }
  void addIPPacketModule(IPPacketModule_c* _IPPacketModule) {
    IPPacketModule = _IPPacketModule;
  }
  void addNat(nat_c* _nat) { nat = _nat; }
  void addTCPSock(TCPSock_c* _TCPSock) { TCPSock = _TCPSock; }

  bool handlePacket(
    char* TCPPacket, int TCPPacketLen, uint32_t sIP, uint32_t dIP
  );
  void sendPacket(
    char* upLayerPacket, int upLayerPacketLen,
    uint32_t sIP, uint16_t sPort, uint32_t dIP, uint16_t dPort,
    uint8_t off=0, uint8_t ttl=0
  );


private:
  // configuration
  IPPacketModule_c* IPPacketModule;
  nat_c* nat;
  TCPSock_c* TCPSock;

  // header
  struct __attribute__ ((packed)) TCPHeader_t {
  uint16_t sport;    // source port 
  uint16_t dport;    // destination port
  uint32_t seq;      // sequence number
  uint32_t ack;      // acknowledgement number
# if __BYTE_ORDER == __LITTLE_ENDIAN
  uint8_t  x2:4;      // (unused)
  uint8_t  off:4;     // data offset
# elif __BYTE_ORDER == __BIG_ENDIAN
  uint8_t  off:4;     // data offset
  uint8_t  x2:4;      // (unused)
# endif
  uint8_t  flags;
  uint16_t rwnd;     // receiving window
  uint16_t checksum;   // checksum
  uint16_t urp;      // urgent pointer
};
  void debug_printPacketHeader(struct TCPHeader_t header);

};














#endif
