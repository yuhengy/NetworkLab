#!/usr/bin/python

import time

import os
import sys
import glob

from mininet.topo import Topo
from mininet.node import OVSBridge
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.cli import CLI

class TCPTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        s1 = self.addSwitch('s1')

        # Delay: 1ms, Packet Drop Rate: 2%
        self.addLink(h1, s1, delay='1ms', loss=2)
        self.addLink(s1, h2)

if __name__ == '__main__':

    topo = TCPTopo()
    net = Mininet(topo = topo, switch = OVSBridge, controller = None, link = TCLink) 

    h1, h2, s1 = net.get('h1', 'h2', 's1')
    h1.cmd('ifconfig h1-eth0 10.0.0.1/24')
    h2.cmd('ifconfig h2-eth0 10.0.0.2/24')

    s1.cmd('scripts/disable_ipv6.sh')


    h1.cmd('scripts/disable_ipv6.sh')
    h2.cmd('scripts/disable_ipv6.sh')
    #h1.cmd('scripts/disable_offloading.sh && scripts/disable_tcp_rst.sh')
    h2.cmd('scripts/disable_offloading.sh && scripts/disable_tcp_rst.sh')
    # XXX: If you want to run user-level stack, you should execute 
    # disable_[arp,icmp,ip_forward].sh first. 
    #h1.cmd('./scripts/disable_arp.sh && ./scripts/disable_icmp.sh && ./scripts/disable_ip_forward.sh')
    h2.cmd('./scripts/disable_arp.sh && ./scripts/disable_icmp.sh && ./scripts/disable_ip_forward.sh')

    net.start()
    #CLI(net)
    h1.cmd('tshark -a duration:30 -w /STEP3-wiresharkOutput-refH1Server.pcapng > result/STEP3-tsharkOutput-refH1Server.log 2>&1 &')
    h2.cmd('tshark -a duration:30 -w /STEP3-wiresharkOutput-myH2Client.pcapng > result/STEP3-tsharkOutput-myH2Client.log 2>&1 &')
    time.sleep(20)

    h1.cmd("stdbuf -oL -eL python build/tcp_stack-BIGFILE.py server 10001 > result/STEP3-refH1Server.txt 2>&1 &")
    #h1.cmd("stdbuf -oL -eL ./build/tcp_stack server 10001 > result/STEP3-myH1Server.txt 2>&1 &")
    time.sleep(1)
    #h2.cmd("python build/tcp_stack-BIGFILE.py client 10.0.0.1 10001 > result/STEP3-refH2Client.txt 2>&1 &")
    h2.cmd("stdbuf -oL -eL ./build/tcp_stack client 0x0a000001 10001 > result/STEP3-myH2Client.txt 2>&1 &")
    time.sleep(39)

    h1.cmd('mv /STEP3-wiresharkOutput-refH1Server.pcapng result/')
    h2.cmd('mv /STEP3-wiresharkOutput-myH2Client.pcapng result/')
    h2.cmd('diff mininet/client-input.dat mininet/server-output.dat > result/STEP3-diff.txt 2>&1')

    net.stop()
