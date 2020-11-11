#ifndef __IP_H__
#define __IP_H__

#include <stdint.h>

class IPPacketModule_c {
public:
  IPPacketModule_c();

  void readIPPacket(char *packet, int len);
  void handleCurrentIPPacket();

  void debug_printCurrentPacket();


private:
  struct IPHeader_t {
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t saddr;
    uint32_t daddr;
  } IPHeader;

};



#endif
