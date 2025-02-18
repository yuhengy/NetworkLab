#!/usr/bin/python

import os
import sys
import time
import glob

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.cli import CLI

script_deps = [ 'ethtool' ]

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

# Mininet will assign an IP address for each interface of a node 
# automatically, but hub or switch does not need IP address.
def clearIP(n):
    for iface in n.intfList():
        n.cmd('ifconfig %s 0.0.0.0' % (iface))

class BroadcastTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        s1 = self.addHost('s1')

        self.addLink(h1, s1, bw=20)
        self.addLink(h2, s1, bw=10)
        self.addLink(h3, s1, bw=10)

if __name__ == '__main__':
    check_scripts()

    topo = BroadcastTopo()
    net = Mininet(topo = topo, link = TCLink, controller = None) 

    h1, h2, h3, s1 = net.get('h1', 'h2', 'h3', 's1')
    h1.cmd('ifconfig h1-eth0 10.0.0.1/8')
    h2.cmd('ifconfig h2-eth0 10.0.0.2/8')
    h3.cmd('ifconfig h3-eth0 10.0.0.3/8')
    clearIP(s1)

    for h in [ h1, h2, h3, s1 ]:
        h.cmd('./scripts/disable_offloading.sh')
        h.cmd('./scripts/disable_ipv6.sh')

    net.start()
    s1.cmd('stdbuf -oL -eL src/switch > result/switchOutput.log 2>&1 &')
    time.sleep(1)
    h1.cmd('ping -c 5 10.0.0.2 > result/STEP1-pingSuccess.log 2>&1')

    ## STEP2 perf performance
    print("STEP2 perf performance")
    h2.cmd('stdbuf -oL -eL iperf -s > result/STEP2-a-H2Server.log 2>&1 &')
    h3.cmd('stdbuf -oL -eL iperf -s > result/STEP2-a-H3Server.log 2>&1 &')
    time.sleep(1)
    h1.cmd('iperf -n 10M -c 10.0.0.2 > result/STEP2-a-H1ClientToH2.log 2>&1 &')
    h1.cmd('iperf -n 10M -c 10.0.0.3 > result/STEP2-a-H1ClientToH3.log 2>&1')
    time.sleep(10)

    h1.cmd('stdbuf -oL -eL iperf -s > result/STEP2-b-H1Server.log 2>&1 &')
    time.sleep(1)
    h2.cmd('iperf -n 10M -c 10.0.0.1 > result/STEP2-b-H2ClientToH1.log 2>&1 &')
    h3.cmd('iperf -n 10M -c 10.0.0.1 > result/STEP2-b-H3ClientToH1.log 2>&1')
    time.sleep(10)
    
    net.stop()
