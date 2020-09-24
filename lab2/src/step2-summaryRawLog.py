#!/usr/bin/python3



import sys
import numpy as np
import json

sys.path.append('src/lib')
from getName import *
from config import *


def getSpeed(bw, delay, block, trialID):
  fname = getLogName(bw, delay, block, trialID)
  with open(fname, 'r') as f:
    lines = f.readlines()
    last_line = []
    i = -1
    while (len(last_line) <= 1):
      last_line = lines[i]
      i -= 1
    speed = float(last_line.split(" ")[2][1:])
    scale = last_line.split(" ")[3][0]

    if scale == 'K':
      speed = speed / 1000
    elif scale == 'M':
      speed = speed
    elif scale == 'G':
      speed = speed * 1000
    else:
      print("error: check the speed.")
      exit() 

    #print(speed)
  return speed

def getAvgSpeed(bw, delay, block):
  speed_all = []
  for trialID in range(1, 1+config['trial']):
    speed_all.append(getSpeed(bw, delay, block, trialID))
  return np.mean(speed_all)

def getDataToPlot():
  summaryResult = dict()
  dataToPlot = dict()
  for delay in config['delay_all']:
    dataToPlot_onecurve = dict()
    for block in config['block_all']:
      onecurve = []
      for bw in config['bw_all']:
        onecurve.append(getAvgSpeed(bw, delay, block))
      dataToPlot_onecurve['blockSize=' + block] = onecurve
    dataToPlot['delay=' + delay] = dataToPlot_onecurve

  summaryResult = dict()
  summaryResult['dataToPlot'] = dataToPlot
  summaryResult['config'] = config

  print('-------------- SUMMAY RESULT--------------')
  print(summaryResult)
  with open(summayResultFile, "w") as f:
    json.dump(summaryResult, f)




if __name__ == "__main__":
  if len(sys.argv) <= 0:
    print("0 arguments needed!")
    exit() 
  getDataToPlot()
