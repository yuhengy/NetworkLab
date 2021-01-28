
import sys
import json
import matplotlib.pyplot as plt



cwndLogFile = sys.argv[1]



with open(cwndLogFile,'r') as f:
  cwndLog = json.load(f)


fig = plt.figure(figsize=(32, 8))
plt.plot([t/1000000 for t in cwndLog["timeStamp"]], cwndLog["cwnd"])


plt.xlabel("Time Stamp (s)")
plt.ylabel("cwnd (number of packages)")
plt.title("howCwndChange")
plt.savefig("howCwndChange.png", bbox_inches="tight")
plt.cla()