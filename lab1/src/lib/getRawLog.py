#!/usr/bin/python

from mininet.net import Mininet
from mininet.topo import Topo
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.node import OVSBridge

import time
import sys
import os

from getName import *

class MyTopo(Topo):
    def build(self, bw=10, delay='10ms', cpu=-1, queueSize=-1):

      if cpu != -1:
        h1 = self.addHost('h1', cpu=cpu)
        h2 = self.addHost('h2', cpu=cpu)
      else:
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')

      if queueSize != -1:
        self.addLink(h1, h2, bw=bw, delay=delay, max_queue_size=queueSize)
      else:
        self.addLink(h1, h2, bw=bw, delay=delay)

def test(bw=10, delay='10ms', block='1M', trial = 5, skipExistedLog = False, cpu=-1, queueSize=-1):
  dirName = getDirName(bw, delay, block)
  sendName = dirName + '/send-' + getFileName(bw, delay, block)
  receiveName = dirName + '/receive-' + getFileName(bw, delay, block)
  topo = MyTopo(bw=bw, delay=delay, cpu=cpu, queueSize=cpu)
  net = Mininet(topo = topo, switch = OVSBridge, link = TCLink, controller=None)

  net.start()
  h1 = net.get('h1')
  h2 = net.get('h2')
  h1.cmd('mkdir -p ' + dirName)
  h2.cmd('python -m SimpleHTTPServer 80 &')
  h2.cmd('dd if=/dev/zero of=' + sendName +' bs='+ block +' count=1')
  time.sleep(1)

  for trialID in range(1, 1+trail):
    logName = getLogName(bw, delay, block, trialID)
    if skipExistedLog:
      if os.path.isfile(logName):
        print("Skip file `" + logName + "` becuase it exists.")
        continue
    h1.cmd('wget -O ' + receiveName + ' http://10.0.0.2/' + sendName + ' > ' + logName + ' 2>&1')
    h1.cmd('rm ' + receiveName)

  h2.cmd('rm ' + sendName)
  h2.cmd('kill %python')
  net.stop()


if __name__ == "__main__":
  if len(sys.argv) <= 7:
    print("7 arguments needed!")
    exit() 
  bw = int(sys.argv[1])
  delay = sys.argv[2]
  block = sys.argv[3]
  trail = int(sys.argv[4])
  skipExistedLog = sys.argv[5] == 'True'
  cpu = float(sys.argv[6])
  queueSize = int(sys.argv[7])

  test(bw, delay, block, trail, skipExistedLog, cpu, queueSize)

