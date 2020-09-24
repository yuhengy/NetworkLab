#!/usr/bin/python3

import sys
import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

sys.path.append('src/lib')
from config import *



marker_all = ("o", "^", "x")
color_all = ("orange", "green", "blue")
linestyle_all = ("--", ":", '-.')
titleLabel_all = ("a", "b", "c", "d")



if __name__ == "__main__":

  with open(summayResultFile,'r') as f:
    dataToPlot = json.load(f)['dataToPlot']

  gs = gridspec.GridSpec(2, 2, width_ratios=[1,1], height_ratios=[1,1])
  fig = plt.figure(figsize=(10, 6))

  for index, ((subtitle, subData), titleLabel) in enumerate(zip(dataToPlot.items(), titleLabel_all)):
    ax = plt.subplot(gs[index])
    ax.plot(np.array(config['bw_all']) / (config['bw_all'][0]), np.array(config['bw_all']) / (config['bw_all'][0]), color='red', linestyle='-', label='equal improvement')
    for index2, ((curveLabel, curveData), marker, color, linestyle) in enumerate(zip(subData.items(), marker_all, color_all, linestyle_all)):
      ax.plot(np.array(config['bw_all'])/(config['bw_all'][0]), np.array(curveData) / (curveData[0]), marker=marker, color=color, linestyle=linestyle, label=curveLabel)
      if index2 == 0:
        for x, y in zip(list(np.array(config['bw_all'])/(config['bw_all'][0])), list(np.array(curveData) / (curveData[0]))):
          ax.text(x, y, "BW=%dM\nFCT=%.2fs"%(x*config['bw_all'][0], float(config['block_all'][0][:-1]) / (y*curveData[0])), fontsize=6)
    
    ax.set_title('('+titleLabel+')'+subtitle, {'fontweight':'bold'})
    ax.set_xscale("log")
    ax.set_yscale("log")
    if index==2 or index==3:
      ax.set_xlabel("Relative Bandwith Improvement")
    if index==0 or index==2:
      ax.set_ylabel("Relative FCT Improvement")
    ax.legend(prop = {'size':9})

  plt.savefig("plot/Result" + ".pdf", bbox_inches="tight")














