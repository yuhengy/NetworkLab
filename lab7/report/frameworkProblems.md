# Why old framework is messy?
Generally, I prefer C++, which can be easy to read, and sothat we can focus on understanding the protocal and network principals. Instead of struggling with the messy code.

All following problems are generally small for each. But they are like grammar errors in an article, which is unrelated with the topic, but makes it hard to read.

Summary these problems are really out of students' responsibility, considering the credits of this course (I would have prefered to use these time on other things, honestly. But, the poor framework should not continue to waste more students' time.

## ip part
+ action scope: in `ip.h`, all macro is global public, which preovides no abstraction, sothat, I do not know their action scope, i.e. can other file use `LE_IP_FMT_STR`?
+ duplicated code: can `LE_IP_FMT_STR` be replaced by general bigend to littleend ?
+ naming: `ip_init_hdr` can mislead to init in ustack_init. Change to create_ip_header.
+ complex file structure: should merge `ip.c` and `ip_base.c`
+ [significant] bad abstraction: input packet should be splitted out of ether header. To achieve this, we need a ether handler to return the ip packet. This is very problemtic because the key of whole network architecture is that, lower layer provides service to higher layer, and higher layer functions (i.e. IP handler) should know nothing about lower layer features (i.e. ether header length). More specificly, `ifaceReceive` call `etherHandle`, call `ipHandle`, call `etherSend`, call `ifaceSend`.
+ performance is unnecessary: in `packet_to_ip_hdr`, you should not always transfer addr, make a copy will cut the long dependence chain and provide a better adstraction.
+ easy to startup: do not init unimplemented with printing TODO, try init it with printing all input. This is helpful for us to know what this function will do.
+ bad abstraction: when we save netend, and when we save hostend? We should only use netend when data is in the packet. We should change data into hostend whenever we copy the packet data into our classes. Moreover, why not baidu `change data end` and find a better changeEnd fucntion?
+ bug: in `ip.h`, `iphdr`, ihl and version do not depend on endian.
+ bad abstraction: rtable is tightly related with IPModule, so, rtable should not be global visible, sothat people will be clear that we only need to consider rtable in IPModule.
+ consistent: it is important to know the difference of handling in this layer, and upload this packet to upper layer. Sothat, this can be consistent with host network architecture.


## arp part
+ complex file structure: should merge arp and arpcache, because arpcache is only used by arp. Otherwise, people will unnecessarily struggle thinking where arpcache is used.

## iface part
+ bug: in `packet.h`, `addr.sll_protocol = htons(ETH_P_ARP);` is fixed into ARP.

## basic
+ bad abstraction: Generally, abstraction trades details (which make mind clearer) to get benefits of thinking higher level (which is easy to memorize). However, in `types.h`, give `uint8_t` a new name `u8` will only lose details and cannot provide the benefits.
+ naming: in `list.h`, please not use name `new`, which is uncompatible with C++.
+ consistent: mac addr should also be store as uint instead of char[]. So that, all ip, mask, mac, ttl, etc. should change endian. 
+ bad abstraction: at the top of IP/ARP, I add a virtual application layer to decide what packet IP/ARP will send. This virtual application can keep IP/ARP holding a interface to upper layer, which is a good abstraction of them.


## good part
+ less copy: all modules handle the packet pointing to the same memory.
