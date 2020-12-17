#!/usr/bin/python

import time

from mininet.node import OVSBridge
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI

class NATTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        s1 = self.addSwitch('s1')
        n1 = self.addHost('n1')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')
        s2 = self.addSwitch('s2')
        n2 = self.addHost('n2')

        self.addLink(h1, s1)
        self.addLink(h2, s1)
        self.addLink(n1, s1)
        self.addLink(n1, n2)
        self.addLink(h3, s2)
        self.addLink(h4, s2)
        self.addLink(n2, s2)

if __name__ == '__main__':
    topo = NATTopo()
    net = Mininet(topo = topo, switch = OVSBridge, controller = None) 

    h1, h2, s1, n1, h3, h4, s2, n2 = net.get('h1', 'h2', 's1', 'n1', 'h3', 'h4', 's2', 'n2')

    h1.cmd('ifconfig h1-eth0 10.21.0.1/16')
    h1.cmd('route add default gw 10.21.0.254')

    h2.cmd('ifconfig h2-eth0 10.21.0.2/16')
    h2.cmd('route add default gw 10.21.0.254')

    n1.cmd('ifconfig n1-eth0 10.21.0.254/16')
    n1.cmd('ifconfig n1-eth1 159.226.39.43/24')

    n2.cmd('ifconfig n2-eth0 159.226.39.123/24')
    n2.cmd('ifconfig n2-eth1 10.21.0.254/16')

    h3.cmd('ifconfig h3-eth0 10.21.0.1/16')
    h3.cmd('route add default gw 10.21.0.254')

    h4.cmd('ifconfig h4-eth0 10.21.0.2/16')
    h4.cmd('route add default gw 10.21.0.254')

    for h in (h1, h2, h3, h4):
        h.cmd('./scripts/disable_offloading.sh')
        h.cmd('./scripts/disable_ipv6.sh')

    for s in (s1, s2):
        s.cmd('./scripts/disable_ipv6.sh')

    for n in (n1, n2):
        n.cmd('./scripts/disable_arp.sh')
        n.cmd('./scripts/disable_icmp.sh')
        n.cmd('./scripts/disable_ip_forward.sh')
        n.cmd('./scripts/disable_ipv6.sh')

    net.start()
    h1.cmd('tshark -a duration:6 -w /STEP3-wiresharOutput-h1.pcapng > result/STEP3-tsharkOutput-h1.log 2>&1 &')
    h2.cmd('tshark -a duration:6 -w /STEP3-wiresharOutput-h2.pcapng > result/STEP3-tsharkOutput-h2.log 2>&1 &')
    h3.cmd('tshark -a duration:6 -w /STEP3-wiresharOutput-h3.pcapng > result/STEP3-tsharkOutput-h3.log 2>&1 &')
    h4.cmd('tshark -a duration:6 -w /STEP3-wiresharOutput-h4.pcapng > result/STEP3-tsharkOutput-h4.log 2>&1 &')
    time.sleep(1)

    n1.cmd("stdbuf -oL -eL ./build/nat mininet/exp3-n1.conf > result/STEP3-nat-n1.txt 2>&1 &")
    n2.cmd("stdbuf -oL -eL ./build/nat mininet/exp3-n2.conf > result/STEP3-nat-n2.txt 2>&1 &")
    #n1.cmd("stdbuf -oL -eL ./build/nat-reference mininet/exp3-n1.conf > result/STEP3-nat-n1.txt 2>&1 &")
    #n2.cmd("stdbuf -oL -eL ./build/nat-reference mininet/exp3-n2.conf > result/STEP3-nat-n2.txt 2>&1 &")
    time.sleep(1)

    h1.cmd("stdbuf -oL -eL python ./mininet/http_server.py > result/STEP3-h1server.txt 2>&1 &")
    h2.cmd("stdbuf -oL -eL python ./mininet/http_server.py > result/STEP3-h2server.txt 2>&1 &")
    time.sleep(1)

    h3.cmd("wget http://159.226.39.43:8000 -O result/STEP3-h3clientToh1-index.html > result/STEP3-h3clientToh1.txt 2>&1 &")
    h3.cmd("wget http://159.226.39.43:8001 -O result/STEP3-h3clientToh2-index.html > result/STEP3-h3clientToh2.txt 2>&1 &")
    h4.cmd("wget http://159.226.39.43:8000 -O result/STEP3-h4clientToh1-index.html > result/STEP3-h4clientToh1.txt 2>&1 &")
    h4.cmd("wget http://159.226.39.43:8001 -O result/STEP3-h4clientToh2-index.html > result/STEP3-h4clientToh2.txt 2>&1 &")
    time.sleep(5)

    h3.cmd('mv /STEP3-wiresharOutput-h1.pcapng result/')
    h3.cmd('mv /STEP3-wiresharOutput-h2.pcapng result/')
    h3.cmd('mv /STEP3-wiresharOutput-h3.pcapng result/')
    h4.cmd('mv /STEP3-wiresharOutput-h4.pcapng result/')

    #CLI(net)
    net.stop()
