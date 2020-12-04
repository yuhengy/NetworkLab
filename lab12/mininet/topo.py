#!/usr/bin/python

import os
import sys
import time
import glob

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI

script_deps = [ 'ethtool', 'arptables', 'iptables' ]

def check_scripts():
    dir = os.path.abspath(os.path.dirname(sys.argv[0]))
    
    for fname in glob.glob(dir + '/' + 'scripts/*.sh'):
        if not os.access(fname, os.X_OK):
            print '%s should be set executable by using `chmod +x $script_name`' % (fname)
            sys.exit(1)

    for program in script_deps:
        found = False
        for path in os.environ['PATH'].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
                found = True
                break
        if not found:
            print '`%s` is required but missing, which could be installed via `apt` or `aptitude`' % (program)
            sys.exit(2)

class MOSPFTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        r1 = self.addHost('r1')
        r2 = self.addHost('r2')
        r3 = self.addHost('r3')
        r4 = self.addHost('r4')

        self.addLink(h1, r1)
        self.addLink(r1, r2)
        self.addLink(r1, r3)
        self.addLink(r2, r4)
        self.addLink(r3, r4)
        self.addLink(r4, h2)

if __name__ == '__main__':
    check_scripts()

    topo = MOSPFTopo()
    net = Mininet(topo = topo, controller = None) 

    h1, h2, r1, r2, r3, r4 = net.get('h1', 'h2', 'r1', 'r2', 'r3', 'r4')
    h1.cmd('ifconfig h1-eth0 10.0.1.11/24')

    r1.cmd('ifconfig r1-eth0 10.0.1.1/24')
    r1.cmd('ifconfig r1-eth1 10.0.2.1/24')
    r1.cmd('ifconfig r1-eth2 10.0.3.1/24')

    r2.cmd('ifconfig r2-eth0 10.0.2.2/24')
    r2.cmd('ifconfig r2-eth1 10.0.4.2/24')

    r3.cmd('ifconfig r3-eth0 10.0.3.3/24')
    r3.cmd('ifconfig r3-eth1 10.0.5.3/24')

    r4.cmd('ifconfig r4-eth0 10.0.4.4/24')
    r4.cmd('ifconfig r4-eth1 10.0.5.4/24')
    r4.cmd('ifconfig r4-eth2 10.0.6.4/24')

    h2.cmd('ifconfig h2-eth0 10.0.6.22/24')

    h1.cmd('route add default gw 10.0.1.1')
    h2.cmd('route add default gw 10.0.6.4')

    for h in (h1, h2):
        h.cmd('./scripts/disable_offloading.sh')
        h.cmd('./scripts/disable_ipv6.sh')

    for r in (r1, r2, r3, r4):
        r.cmd('./scripts/disable_arp.sh')
        r.cmd('./scripts/disable_icmp.sh')
        r.cmd('./scripts/disable_ip_forward.sh')
        r.cmd('./scripts/disable_ipv6.sh')

    net.start()
    for index, r in enumerate((r1, r2, r3, r4)):
        #r.cmd('stdbuf -oL -eL ./build/mospfd-reference > result-reference/STEP1-r%d.txt 2>&1 &' % (index+1))
        r.cmd('stdbuf -oL -eL ./build/mospfd > result/STEP1-r%d.txt 2>&1 &' % (index+1))
    
    time.sleep(40)
    #h1.cmd("traceroute 10.0.6.22 > result/STEP2-firstPing.txt 2>&1")

    #net.configLinkStatus("r2", "r4", "down")

    #time.sleep(30)
    #h1.cmd("traceroute 10.0.6.22 > result/STEP2-secondPing.txt 2>&1")
    net.stop()
