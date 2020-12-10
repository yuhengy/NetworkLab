

import sys


def parseDataSet(fileName):
  dataSet = []


  with open(fileName, 'r') as f:
    lines = f.readlines()
    for line in lines:
      if len(line.split()) !=0:
        IPRawStr, mask, iface = line.split()
        IPStr = ""
        for byte in IPRawStr.split("."):
          IPStr += format(int(byte), "b").zfill(8)
        dataSet.append({"IPStr": IPStr, "mask": int(mask), "iface": int(iface)})

  return dataSet



if __name__ == "__main__":
  fileName = sys.argv[1]

  print(parseDataSet(fileName))

