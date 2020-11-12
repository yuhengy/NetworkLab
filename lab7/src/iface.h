#ifndef __IFACE_H__
#define __IFACE_H__

#include <stdint.h>

class iface_c {
public:
  iface_c(int _fd, int _index, uint64_t _mac,
    uint32_t _ip, uint32_t _mask, char* _name, char* _ip_str);
  
  void handlePacket(char *packet, int len);
  void sendPacket(char *packet, int len);

  void debug_printiface();
  int      getFd()  { return info.fd;  }
  uint32_t getIP()  { return info.ip;  }
  uint64_t getMac() { return info.mac; }


private:
  struct __attribute__ ((packed)) ifaceInfo_t {
    int fd;
    int index;
    uint64_t mac:48;
    uint32_t ip;
    uint32_t mask;
    char name[16];
    char ip_str[16];
  } info;

};

#endif
