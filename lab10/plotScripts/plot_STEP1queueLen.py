

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

#--------config begin--------
color_all = ("orange", "green", "purple", "red", "blue", "brown")
#--------config end--------

def parseFileGetCwndList(queueLenFile):
  time = []
  queueLen = []
  with open(queueLenFile, 'r') as f:
    lines = f.readlines()[:-1]
    for line in lines:
      # 1. add time
      time.append(float(line.split()[0][:-1]))

      # 2. add queueLen
      queueLen.append(int(line.split()[1]))

  timeOffset = np.array(time) - time[0]
  return timeOffset, queueLen




if __name__ == "__main__":
  args = sys.argv[1:]
  queueLenFile_all = args[:int(len(args)/2)][::-1]
  print("queueLenFile_all : " + str(queueLenFile_all))
  queueSize_all = [int(i) for i in args[int(len(args)/2):]][::-1]
  print("queueSize_all : " + str(queueSize_all))

  plt.figure()

  for queueLenFile, queueSize, color in zip(queueLenFile_all, queueSize_all, color_all):
    timeOffset, queueLen = parseFileGetCwndList(queueLenFile)
    #print(timeOffset)
    #print(queueLen)
    plt.plot(timeOffset, queueLen, color=color, label="Queue size = %d" % queueSize)

  plt.yscale("log")

  plt.xlabel("Seconds")
  plt.ylabel("QueueLen (# Packet)")
  plt.legend()
  plt.savefig("plots/STEP1-queueLen.png", bbox_inches="tight")








