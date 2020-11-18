

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

#--------config begin--------
color_all = ("orange", "green", "purple", "red", "blue", "brown")
#--------config end--------

def parseFileGetCwndList(cwndFile):
  time = []
  cwnd = []
  with open(cwndFile, 'r') as f:
    lines = f.readlines()[:-1]
    for line in lines:
      # 1. add time
      time.append(float(line.split()[0][:-1]))

      # 2. add cwnd
      for word in line.split():
        succeed = False
        if (len(word) > 4 and word[0:4] == "cwnd"):
          cwnd.append(int(word[5:]))
          succeed = True
          break

      # 3. check succeed
      if not succeed:
        print("Error: cannot find a cwnd for a time, use nearest cwnd instead")
        cwnd.append(cwnd[-1])

  timeOffset = np.array(time) - time[0]
  return timeOffset, cwnd




if __name__ == "__main__":
  args = sys.argv[1:]
  cwndFile_all = args[:int(len(args)/2)][::-1]
  print("cwndFile_all : " + str(cwndFile_all))
  queueSize_all = [int(i) for i in args[int(len(args)/2):]][::-1]
  print("queueSize_all : " + str(queueSize_all))

  plt.figure()

  for cwndFile, queueSize, color in zip(cwndFile_all, queueSize_all, color_all):
    timeOffset, cwnd = parseFileGetCwndList(cwndFile)
    #print(timeOffset)
    #print(cwnd)
    plt.plot(timeOffset, cwnd, color=color, label="Queue size = %d" % queueSize)

  plt.yscale("log")

  plt.xlabel("Seconds")
  plt.ylabel("CWND (KB)")
  plt.legend()
  plt.savefig("plots/STEP1-cwnd.png", bbox_inches="tight")








