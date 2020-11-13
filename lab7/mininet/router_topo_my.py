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

class RouterTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        r1 = self.addHost('r1')
        r2 = self.addHost('r2')
        h2 = self.addHost('h2')

        self.addLink(h1, r1)
        self.addLink(r1, r2)
        self.addLink(r2, h2)

if __name__ == '__main__':
    check_scripts()

    topo = RouterTopo()
    net = Mininet(topo = topo, controller = None) 

    h1, r1, r2, h2 = net.get('h1', 'r1', 'r2', 'h2')
    h1.cmd('ifconfig h1-eth0 10.0.1.11/24')
    h2.cmd('ifconfig h2-eth0 10.0.2.22/24')

    h1.cmd('route add default gw 10.0.1.1')
    h2.cmd('route add default gw 10.0.2.1')

    r1.cmd('ifconfig r1-eth0 10.0.1.1/24')
    r1.cmd('ifconfig r1-eth1 10.0.3.1/24')
    r1.cmd('route add -net 10.0.2.0 netmask 255.255.255.0 gw 10.0.3.2 dev r1-eth1')
    r2.cmd('ifconfig r2-eth0 10.0.3.2/24')
    r2.cmd('ifconfig r2-eth1 10.0.2.1/24')
    r2.cmd('route add -net 10.0.1.0 netmask 255.255.255.0 gw 10.0.3.1 dev r2-eth0')

    for n in (h1, r1, r2, h2):
        n.cmd('./scripts/disable_offloading.sh')
        n.cmd('./scripts/disable_ipv6.sh')

    for n in (r1, r2):
        n.cmd('./scripts/disable_arp.sh')
        n.cmd('./scripts/disable_icmp.sh')
        n.cmd('./scripts/disable_ip_forward.sh')
        n.cmd('./scripts/disable_ipv6.sh')

    net.start()
    h1.cmd('tshark -a duration:10 -w /wiresharOutput-h1.pcapng > result/STEP2-tsharkOutput-h1.log 2>&1 &')
    h2.cmd('tshark -a duration:10 -w /wiresharOutput-h2.pcapng > result/STEP2-tsharkOutput-h2.log 2>&1 &')
    time.sleep(1)

    r1.cmd('stdbuf -oL -eL ./build/router > result/STEP2-router1.txt 2>&1 &')
    r2.cmd('stdbuf -oL -eL ./build/router > result/STEP2-router2.txt 2>&1 &')
    #r1.cmd('stdbuf -oL -eL ./src/router-reference > result/STEP2-router1.txt 2>&1 &')
    #r2.cmd('stdbuf -oL -eL ./src/router-reference > result/STEP2-router2.txt 2>&1 &')
    time.sleep(1)

    h1.cmd('ping -c 5 10.0.1.1 > result/STEP2a-pingR1.txt 2>&1')
    h1.cmd('ping -c 5 10.0.3.2 > result/STEP2b-pingR2.txt 2>&1')
    h1.cmd('ping -c 5 10.0.2.22 > result/STEP2c-pingH2.txt 2>&1')

    h1.cmd('echo \'***********************************\' > result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'**************ttl = 1**************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'***********************************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('ping -c 1 -t 1 10.0.2.22 >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'***********************************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'**************ttl = 2**************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'***********************************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('ping -c 1 -t 2 10.0.2.22 >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'***********************************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'**************ttl = 3**************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('echo \'***********************************\' >> result/STEP2d-tracerouteH2.txt 2>&1')
    h1.cmd('ping -c 1 -t 3 10.0.2.22 >> result/STEP2d-tracerouteH2.txt 2>&1')

    time.sleep(10)
    h1.cmd('mv /wiresharOutput-h1.pcapng result/')
    h2.cmd('mv /wiresharOutput-h2.pcapng result/')

    #CLI(net)
    net.stop()
