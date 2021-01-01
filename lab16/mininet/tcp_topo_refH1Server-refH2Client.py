#!/usr/bin/python

import time

import os
import sys
import glob

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.link import TCLink
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

class TCPTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')

        self.addLink(h1, h2, delay='10ms')

if __name__ == '__main__':
    check_scripts()

    topo = TCPTopo()
    net = Mininet(topo = topo, link = TCLink, controller = None) 

    h1, h2 = net.get('h1', 'h2')
    h1.cmd('ifconfig h1-eth0 10.0.0.1/24')
    h2.cmd('ifconfig h2-eth0 10.0.0.2/24')


    h1.cmd('scripts/disable_ipv6.sh')
    h2.cmd('scripts/disable_ipv6.sh')
    #h1.cmd('scripts/disable_offloading.sh && scripts/disable_tcp_rst.sh')
    #h2.cmd('scripts/disable_offloading.sh && scripts/disable_tcp_rst.sh')
    # XXX: If you want to run user-level stack, you should execute 
    # disable_[arp,icmp,ip_forward].sh first. 
    #h1.cmd('./scripts/disable_arp.sh && ./scripts/disable_icmp.sh && ./scripts/disable_ip_forward.sh')
    #h2.cmd('./scripts/disable_arp.sh && ./scripts/disable_icmp.sh && ./scripts/disable_ip_forward.sh')

    net.start()
    #CLI(net)
    h1.cmd('tshark -a duration:30 -w /STEP0-wiresharkOutput-refH1Server.pcapng > result/STEP0-tsharkOutput-refH1Server.log 2>&1 &')
    h2.cmd('tshark -a duration:30 -w /STEP0-wiresharkOutput-myH2Client.pcapng > result/STEP0-tsharkOutput-myH2Client.log 2>&1 &')
    time.sleep(20)

    h1.cmd("python build/tcp_stack.py server 10001 > result/STEP0-refH1Server.txt 2>&1 &")
    #h1.cmd("stdbuf -oL -eL ./build/tcp_stack server 10001 > result/STEP0-myH1Server.txt 2>&1 &")
    time.sleep(1)
    h2.cmd("python build/tcp_stack.py client 10.0.0.1 10001 > result/STEP0-refH2Client.txt 2>&1 &")
    #h2.cmd("stdbuf -oL -eL ./build/tcp_stack client 0x0a000001 10001 > result/STEP0-myH2Client.txt 2>&1 &")
    time.sleep(39)

    h1.cmd('mv /STEP0-wiresharkOutput-refH1Server.pcapng result/')
    h2.cmd('mv /STEP0-wiresharkOutput-myH2Client.pcapng result/')

    net.stop()
