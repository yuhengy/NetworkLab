#!/usr/bin/python3

def getDirName(bw, delay, block):
  return '../data/rawData-delay' + delay

def getFileName(bw, delay, block):
  return 'block' + block + '-bw%d'%bw + '.dat'

def getLogName(bw, delay, block, trialID):
  dirName = getDirName(bw, delay, block)
  return dirName + '/block' + block + '-bw%d'%bw + "trial%d"%trialID + '.log'