#!/usr/bin/python

import os
import sys
import time
import glob

from mininet.topo import Topo
from mininet.net import Mininet
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

def clearIP(n):
    for iface in n.intfList():
        n.cmd('ifconfig %s 0.0.0.0' % (iface))

class RingTopo(Topo):
    def build(self):
        b1 = self.addHost('b1')
        b2 = self.addHost('b2')
        b3 = self.addHost('b3')
        b4 = self.addHost('b4')
        b5 = self.addHost('b5')
        b6 = self.addHost('b6')
        b7 = self.addHost('b7')

        self.addLink(b1, b2)
        self.addLink(b2, b3)
        self.addLink(b3, b4)
        self.addLink(b4, b5)
        self.addLink(b5, b6)
        self.addLink(b6, b7)
        self.addLink(b7, b1)
        self.addLink(b3, b7)

if __name__ == '__main__':
    check_scripts()

    topo = RingTopo()
    net = Mininet(topo = topo, controller = None) 

    for idx in range(7):
        name = 'b' + str(idx+1)
        node = net.get(name)
        clearIP(node)
        node.cmd('./scripts/disable_offloading.sh')
        node.cmd('./scripts/disable_ipv6.sh')

        # set mac address for each interface
        for port in range(len(node.intfList())):
            intf = '%s-eth%d' % (name, port)
            mac = '00:00:00:00:0%d:0%d' % (idx+1, port+1)

            node.setMAC(mac, intf = intf)

    net.start()
    for idx in range(7):
        name = 'b' + str(idx+1)
        node = net.get(name)
        node.cmd('stdbuf -oL -eL src/stp > result/%s-output.txt 2>&1 &' % name)
        #node.cmd('stdbuf -oL -eL src/stp-reference > result/%s-output.txt 2>&1 &' % name)

    time.sleep(30)
    for idx in range(7):
        name = 'b' + str(idx+1)
        node = net.get(name)
        node.cmd('pkill -SIGTERM stp')
    time.sleep(1)

    net.stop()
