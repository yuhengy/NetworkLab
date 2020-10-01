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
h1.cmd('stdbuf -oL -eL ./build/server > result/test1_myServer_log.log 2>&1 &')
time.sleep(1)
# TEST1a continuous request
h2.cmd('./build/client /test_send.txt result/test1a-1_receive_myClientFromMyServer.txt > result/test1a-1_myClient_log.log 2>&1')
h2.cmd('./build/client /test_send.txt result/test1a-2_receive_myClientFromMyServer.txt > result/test1a-2_myClient_log.log 2>&1')
h2.cmd('./build/client /test_send.txt result/test1a-3_receive_myClientFromMyServer.txt > result/test1a-3_myClient_log.log 2>&1')
time.sleep(1)
# TEST1b parallel request, to check the correctness, all requests finish at the same time.
h2.cmd('./build/client /test_send.txt result/test1b-1_receive_myClientFromMyServer.txt > result/test1b-1_myClient_log.log 2>&1 &')
h2.cmd('./build/client /test_send.txt result/test1b-2_receive_myClientFromMyServer.txt > result/test1b-2_myClient_log.log 2>&1 &')
h2.cmd('./build/client /test_send.txt result/test1b-3_receive_myClientFromMyServer.txt > result/test1b-3_myClient_log.log 2>&1 &')
time.sleep(15)
# TEST1c File not found
h2.cmd('./build/client /test_notexist_send.txt result/test1c_notexist_receive_myClientFromMyServer.txt >> result/test1c_myClient_log.log 2>&1')
net.stop()


topo = MyTopo()
net = Mininet(topo = topo, switch = OVSBridge, link = TCLink, controller = None)
net.start()
h1 = net.get('h1')
h2 = net.get('h2')
# TEST2 wgetClient to myServer
h1.cmd('stdbuf -oL -eL ./build/server > result/test2_myServer_log.log 2>&1 &')
time.sleep(1)
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2a-1_receive_wgetFromMyServer.txt > result/test2a-1_wgetClient_log.log 2>&1')
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2a-2_receive_wgetFromMyServer.txt > result/test2a-2_wgetClient_log.log 2>&1')
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2a-3_receive_wgetFromMyServer.txt > result/test2a-3_wgetClient_log.log 2>&1')
time.sleep(1)
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2b-1_receive_wgetFromMyServer.txt > result/test2b-1_wgetClient_log.log 2>&1 &')
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2b-2_receive_wgetFromMyServer.txt > result/test2b-2_wgetClient_log.log 2>&1 &')
h2.cmd('wget http://10.0.0.1/test_send.txt -O result/test2b-3_receive_wgetFromMyServer.txt > result/test2b-3_wgetClient_log.log 2>&1 &')
time.sleep(15)
# TEST2c File not found
h2.cmd('wget http://10.0.0.1/test_notexist_send.txt -O result/test2c_notexist_receive_wgetFromMyServer.txt >> result/test2c_wgetClient_log.log 2>&1')
net.stop()

topo = MyTopo()
net = Mininet(topo = topo, switch = OVSBridge, link = TCLink, controller = None)
net.start()
h1 = net.get('h1')
h2 = net.get('h2')
# TEST3 myClient to pythonServer
h1.cmd('stdbuf -oL -eL python -m SimpleHTTPServer 80 > result/test3_pythonServer_log.log 2>&1 &')
time.sleep(1)
# TEST3a continuous request
h2.cmd('./build/client /test_send.txt result/test3a-1_receive_myClientFromMyServer.txt > result/test3a-1_myClient_log.log 2>&1')
time.sleep(5)
h2.cmd('./build/client /test_send.txt result/test3a-2_receive_myClientFromMyServer.txt > result/test3a-2_myClient_log.log 2>&1')
time.sleep(5)
h2.cmd('./build/client /test_send.txt result/test3a-3_receive_myClientFromMyServer.txt > result/test3a-3_myClient_log.log 2>&1')
time.sleep(1)
# TEST3c File not found
h2.cmd('./build/client /test_notexist_send.txt result/test3c_notexist_receive_myClientFromMyServer.txt >> result/test3c_myClient_log.log 2>&1')
net.stop()
