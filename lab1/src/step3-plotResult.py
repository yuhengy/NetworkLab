#!/usr/bin/python3

import sys
import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

sys.path.append('lib')
from config import *



marker_all = ("o", "^", "x")
color_all = ("orange", "green", "blue")
linestyle_all = ("--", ":", '-.')



if __name__ == "__main__":

  with open(summayResultFile,'r') as f:
    dataToPlot = json.load(f)['dataToPlot']

  gs = gridspec.GridSpec(2, 2, width_ratios=[1,1], height_ratios=[1,1])
  fig = plt.figure(figsize=(10, 10))

  for index, (subtitle, subData) in enumerate(dataToPlot.items()):
    ax = plt.subplot(gs[index])
    ax.plot(config['bw_all'], np.array(config['bw_all']) / config['bw_all'][0], color='red', linestyle='-', label='equal improvement in FCT and bandwidth')
    for (curveLabel, curveData), marker, color, linestyle in zip(subData.items(), marker_all, color_all, linestyle_all):
      ax.plot(config['bw_all'], np.array(curveData) / curveData[0], marker=marker, color=color, linestyle=linestyle, label=curveLabel)
    
    ax.set_title(subtitle, {'fontweight':'bold'})
    ax.set_xscale("log")
    ax.set_yscale("log")
    if index==2 or index==3:
      ax.set_xlabel("Relative Bandwith Improvement")
    if index==0 or index==2:
      ax.set_ylabel("Relative FCT Improvement")
    ax.legend()

  plt.savefig("../plot/Result" + ".pdf", bbox_inches="tight")














