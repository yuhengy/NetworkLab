#!/usr/bin/python

import time

from mininet.node import OVSBridge
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI

class NATTopo(Topo):
    def build(self):
        s1 = self.addSwitch('s1')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        n1 = self.addHost('n1')

        self.addLink(h1, s1)
        self.addLink(h2, s1)
        self.addLink(n1, s1)
        self.addLink(h3, n1)

if __name__ == '__main__':
    topo = NATTopo()
    net = Mininet(topo = topo, switch = OVSBridge, controller = None) 

    h1, h2, h3, s1, n1 = net.get('h1', 'h2', 'h3', 's1', 'n1')

    h1.cmd('ifconfig h1-eth0 10.21.0.1/16')
    h1.cmd('route add default gw 10.21.0.254')

    h2.cmd('ifconfig h2-eth0 10.21.0.2/16')
    h2.cmd('route add default gw 10.21.0.254')

    n1.cmd('ifconfig n1-eth0 10.21.0.254/16')
    n1.cmd('ifconfig n1-eth1 159.226.39.43/24')

    h3.cmd('ifconfig h3-eth0 159.226.39.123/24')

    for h in (h1, h2, h3):
        h.cmd('./scripts/disable_offloading.sh')
        h.cmd('./scripts/disable_ipv6.sh')

    s1.cmd('./scripts/disable_ipv6.sh')

    n1.cmd('./scripts/disable_arp.sh')
    n1.cmd('./scripts/disable_icmp.sh')
    n1.cmd('./scripts/disable_ip_forward.sh')
    n1.cmd('./scripts/disable_ipv6.sh')

    net.start()
    #CLI(net)

    h1.cmd('tshark -a duration:6 -w /STEP1-wiresharOutput-h1.pcapng > result/STEP1-tsharkOutput-h1.log 2>&1 &')
    h2.cmd('tshark -a duration:6 -w /STEP1-wiresharOutput-h2.pcapng > result/STEP1-tsharkOutput-h2.log 2>&1 &')
    h3.cmd('tshark -a duration:6 -w /STEP1-wiresharOutput-h3.pcapng > result/STEP1-tsharkOutput-h3.log 2>&1 &')
    time.sleep(1)
    
    n1.cmd("stdbuf -oL -eL ./build/nat mininet/exp1.conf > result/STEP1-nat.txt 2>&1 &")
    #n1.cmd("stdbuf -oL -eL ./build/nat-reference mininet/exp1.conf > result/STEP1-nat.txt 2>&1 &")
    time.sleep(1)
    h3.cmd("stdbuf -oL -eL python ./mininet/http_server.py > result/STEP1-h3server.txt 2>&1 &")
    time.sleep(1)
    h1.cmd("wget http://159.226.39.123:8000 -O result/STEP1-h1client-index.html > result/STEP1-h1client.txt 2>&1 &")
    h2.cmd("wget http://159.226.39.123:8000 -O result/STEP1-h2client-index.html > result/STEP1-h2client.txt 2>&1 &")
    
    time.sleep(5)
    h3.cmd('mv /STEP1-wiresharOutput-h1.pcapng result/')
    h3.cmd('mv /STEP1-wiresharOutput-h2.pcapng result/')
    h3.cmd('mv /STEP1-wiresharOutput-h3.pcapng result/')


    #CLI(net)
    net.stop()
