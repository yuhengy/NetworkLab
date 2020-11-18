

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

#--------config begin--------
color_all = ("orange", "green", "purple", "red", "blue", "brown")
#--------config end--------

def parseFileGetCwndList(rttFile):
  time = []
  rtt = []
  with open(rttFile, 'r') as f:
    lines = f.readlines()[:-1]
    if (len(lines) == 0):
      return [], []
    for line in lines:
      # 1. add time
      if line == '\n':
        continue
      time.append(float(line.split()[0][:-1]))

      # 2. add rtt
      for word in line.split():
        succeed = False
        if (len(word) > 4 and word[0:4] == "time"):
          rtt.append(float(word[5:]))
          succeed = True
          break

      # 3. check succeed
      if not succeed:
        print("Warning: cannot find a rtt for a time, use nearest rtt instead")
        rtt.append(rtt[-1])

  timeOffset = np.array(time) - time[0]
  return timeOffset, rtt




if __name__ == "__main__":
  args = sys.argv[1:]
  rttFile_all = args[:int(len(args)/2)][::-1]
  print("rttFile_all : " + str(rttFile_all))
  queueSize_all = [int(i) for i in args[int(len(args)/2):]][::-1]
  print("queueSize_all : " + str(queueSize_all))

  plt.figure()

  for rttFile, queueSize, color in zip(rttFile_all, queueSize_all, color_all):
    timeOffset, rtt = parseFileGetCwndList(rttFile)
    #print(timeOffset)
    #print(rtt)
    plt.plot(timeOffset, rtt, color=color, label="Queue size = %d" % queueSize)

  plt.yscale("log")

  plt.xlabel("Seconds")
  plt.ylabel("RTT (ms)")
  plt.legend()
  plt.savefig("plots/STEP1-rtt.png", bbox_inches="tight")








