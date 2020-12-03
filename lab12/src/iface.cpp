#include "iface.h"

#include <stdio.h>
#include <string.h>

iface_c::iface_c(int _fd, int _index, uint64_t _mac, uint32_t _ip, uint32_t _mask, char* _name, char* _ip_str)
{
  info.fd = _fd;
  info.index = _index;
  info.mac = _mac;
  info.ip = _ip;
  info.mask = _mask;
  memcpy(&(info.name), _name, 16);
  memcpy(&(info.ip_str), _ip_str, 16);
}

void iface_c::debug_printiface()
{
  printf("---------------iface start---------------\n");
  printf("fd:     %d\n"       , info.fd);
  printf("index:  %d\n"       , info.index);
  printf("mac:    0x%012lx\n" , info.mac);
  printf("ip:     0x%08x\n"   , info.ip);
  printf("mask:   0x%08x\n"   , info.mask);
  printf("name:   %s\n"       , info.name);
  printf("ip_str: %s\n"       , info.ip_str);
  printf("^^^^^^^^^^^^^^^iface end^^^^^^^^^^^^^^^\n");

}