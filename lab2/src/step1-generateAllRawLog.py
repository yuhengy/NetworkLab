#!/usr/bin/python3

import os
import sys
from multiprocessing import Pool

sys.path.append('src/lib')
from config import *

def worker(shellCmd):
  os.system(shellCmd)


def generateAllRawLog():
  p = Pool(jobs)
  for delay in config['delay_all']:
    for bw in config['bw_all']:
      for block in config['block_all']:
        ## Note: this is python2
        shellCmd = "sudo python src/lib/getRawLog.py " + str(bw) + ' ' + delay + ' ' + block + ' ' + str(config['trial']) + ' ' + str(skipExistedLog) + ' ' + str(config['cpu']) + ' ' + str(config['queueSize'])
        p.apply_async(worker, args=(shellCmd,))
  p.close()
  p.join()


if __name__ == "__main__":
  generateAllRawLog()