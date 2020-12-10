
import numpy as np
import matplotlib.pyplot as plt

CONFIG = 1

if CONFIG == 1:
  plotName = "Router Lookup Speed up"
  ylable = "Speed up"
  baseTime = 1.935155
  data_y1 = baseTime/np.array([2.023019, 1.498382, 1.170105, 1.226116])
  data_y2 = baseTime/np.array([4.334533, 2.716133, 2.036936, 1.742350])

if CONFIG == 2:
  plotName = "Router Lookup Memory Usage"
  ylable = "Memory percentage"
  baseMem = 529
  data_y1 = np.array([752, 570, 617, 1181])/baseMem
  data_y2 = np.array([785, 637, 687, 1369])/baseMem

if CONFIG == 3:
  plotName = "Router Lookup Speed up - Small Router Table"
  ylable = "Speed up"
  baseTime = None
  data_y1 = baseTime/np.array([])
  data_y2 = baseTime/np.array([])

if CONFIG == 4:
  plotName = "Router Lookup Memory Usage - Small Router Table"
  ylable = "Memory percentage"
  baseMem = None
  data_y1 = np.array([])/baseMem
  data_y2 = np.array([])/baseMem



fig = plt.figure()
plt.plot([1, 2, 4, 8], data_y1, label="with multibit")
plt.plot([1, 2, 4, 8], data_y2, label="with multibit and compress")

plt.xlabel("# of bits")
plt.ylabel(ylable)
plt.legend()
plt.title(plotName)
plt.savefig("plots/" + plotName + ".png", bbox_inches="tight")
plt.cla()
