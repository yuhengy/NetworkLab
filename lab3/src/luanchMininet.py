from mininet.net import Mininet
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.node import OVSBridge

import time

class MyTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        s1 = self.addSwitch('s1')

        self.addLink(h1, s1, bw=10, delay='10ms')
        self.addLink(h2, s1)

topo = MyTopo()
net = Mininet(topo = topo, switch = OVSBridge, link = TCLink, controller = None)
net.start()
h1 = net.get('h1')
h2 = net.get('h2')
h1.cmd('mkdir -p result')

# TEST1 myClient to myServer
h1.cmd('./build/server > result/test1_myServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('./build/client /test_send.txt result/test1_receive_myClientFromMyServer.txt > result/test1_myClient_log.log 2>&1')
time.sleep(1)
h1.cmd('./build/server >> result/test1_myServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('./build/client /test_notexist_send.txt result/test1_notexist_receive_myClientFromMyServer.txt >> result/test1_myClient_log.log 2>&1')
time.sleep(1)

# TEST2 wgetClient to myServer
h1.cmd('./build/server > result/test2_myServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2_receive_wgetFromMyServer.txt > result/test2_wgetClient_log.log 2>&1')
time.sleep(1)
h1.cmd('./build/server >> result/test2_myServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('wget http://10.0.0.1/test_notexist_send.txt -O result/test2_notexist_receive_wgetFromMyServer.txt >> result/test2_wgetClient_log.log 2>&1')
time.sleep(1)

# TEST3 myClient to pythonServer
h1.cmd('python -m SimpleHTTPServer 80 > result/test3_pythonServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('./build/client /test_send.txt result/test3_receive_myClientFromPythonServer.txt > result/test3_myClient_log.log 2>&1')
time.sleep(1)
h2.cmd('./build/client /test_notexist_send.txt result/test3_notexist_receive_myClientFromPythonServer.txt >> result/test3_myClient_log.log 2>&1')
time.sleep(1)



net.stop()
