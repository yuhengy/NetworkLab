

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

#--------config begin--------
color_all = ("orange", "green", "purple", "red", "blue", "brown")
bandwidth_all = (100, 10, 1, 50, 1, 100)
#--------config end--------

from plot_STEP1rtt import parseFileGetCwndList


if __name__ == "__main__":
  args = sys.argv[1:]
  rttFile_all = args[:int(len(args)/2)]
  print("rttFile_all : " + str(rttFile_all))
  algo_all = args[int(len(args)/2):]
  print("algo_all : " + str(algo_all))

  plt.figure(figsize=(9.6, 4.8))

  for rttFile, algo, color in zip(rttFile_all, algo_all, color_all):
    timeOffset, rtt = parseFileGetCwndList(rttFile)
    #print(timeOffset)
    #print(rtt)
    plt.plot(timeOffset, rtt, color=color, label="algorithm is " + algo)


  for index, bandwidth in enumerate(bandwidth_all):
    if index != 0:
      plt.axvline(index*10, color="red", ls="--")#, lw=20)
    plt.text(index*10 + 2, 100, "%3d Mbps" % bandwidth, color="red")

  plt.yscale("log")

  plt.xlabel("Seconds")
  plt.ylabel("RTT (ms)")
  plt.legend()
  plt.savefig("plots/STEP2-rtt.png", bbox_inches="tight")








