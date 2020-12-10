
import sys
from guppy import hpy
import time

from parseDataSet import parseDataSet
from basePrefixTree import checkWithBase

#BIT_NUM = 8


def changeToMultibit(dataSet):
  for entry in dataSet:
    IPmultibit = []
    multibit = 0

    for i, bit in enumerate(entry["IPStr"]):
      multibit <<= 1
      multibit += int(bit)
      if (i % BIT_NUM) == (BIT_NUM - 1):
        IPmultibit.append(multibit)
        multibit = 0

    entry["IPmultibit"] = IPmultibit
  return dataSet


class optimizedPrefixTree_c():
  def __init__(self):
      self.root = {"match": False}

  def insertEntry(self, IPmultibit, IPStr, mask, iface):
    maskPadder = (32-mask) % BIT_NUM
    node  = self.root
    insertLog = []
    for multibit, _ in zip(IPmultibit, range(int(mask/BIT_NUM))):
      #if "IPStr" in node.keys():
      #  insertLog.append([multibit, node["match"], node["IPStr"], node["mask"], node["iface"]])
      #else:
      #  insertLog.append([multibit, node["match"]])
      node = node.setdefault(multibit, {"match": False})
    
    if maskPadder == 0:
      nodeList = [node]
    else:
      multibitRemainer = (IPmultibit[int(mask/BIT_NUM)] \
                         >> maskPadder) \
                         << maskPadder
      nodeList = []
      for i in range(2**maskPadder):
        nodeList.append(node.setdefault(multibitRemainer+i, {"match": False}))

    for updatedNode in nodeList:
      if (not updatedNode["match"]) or \
        (updatedNode["match"] and updatedNode["mask"] < mask):

        updatedNode["match"] = True
        updatedNode["IPStr"] = IPStr
        updatedNode["mask"]  = mask
        updatedNode["iface"] = iface

    #print("----------------insertLog----------------")
    #for insertLog_one in insertLog:
    #  print(insertLog_one)
    #print("----------------insertLog----------------")


  def searchIP(self, IPmultibit, IPStr):
    node = self.root
    lastMatchNode = {"match": False, "IPStr": "0", "mask": 0, "iface": 0}

    searchLog = []
    for multibit in IPmultibit:
      #if "IPStr" in node.keys():
      #  searchLog.append([multibit, node["match"], node["IPStr"], node["mask"], node["iface"]])
      #else:
      #  searchLog.append([multibit, node["match"]])
      if multibit not in node:
        break
      node = node[multibit]
      if node["match"] == True:
        lastMatchNode = node

    #print("----------------searchLog----------------")
    #for searchLog_one in searchLog:
    #  print(searchLog_one)
    #print("----------------searchLog----------------")

    return lastMatchNode["match"], lastMatchNode["IPStr"], \
           lastMatchNode["mask"] , lastMatchNode["iface"], searchLog

def generateResult(fileName):

  dataSet = changeToMultibit(parseDataSet(fileName))
  result = []

  timeStart = time.time()
  optimizedPrefixTree = optimizedPrefixTree_c()
  for entry in dataSet:
    optimizedPrefixTree.insertEntry( \
      entry["IPmultibit"], entry["IPStr"], entry["mask"], entry["iface"] \
    )
  timeToCreate = time.time() - timeStart

  #print(optimizedPrefixTree.root)
  memoryUsageMB = int(int((str(hpy().heap())).split()[10]) / 1024 / 1024)

  timeStart = time.time()
  for i, entry in enumerate(dataSet):
    match, IPStr, mask, iface, searchLog = \
      optimizedPrefixTree.searchIP(entry["IPmultibit"], entry["IPStr"])
    result.append({"match": match, "IPStr": IPStr, "mask": int(mask), "iface": int(iface)})

    if not match:
      print("Cannot find %d-th entry: " % (i + 1), entry)
      print("----------------searchLog----------------")
      for searchLog_one in searchLog:
        print(searchLog_one)
      print("----------------searchLog----------------")
      assert(False)

    #print("From ", entry, " --- Find:", match, IPStr, mask, iface)
  timeToSearch = time.time() - timeStart

  return result, timeToCreate, timeToSearch, memoryUsageMB





if __name__ == "__main__":
  fileName = sys.argv[1]
  BIT_NUM = int(sys.argv[2])

  result, timeToCreate, timeToSearch, memoryUsageMB = generateResult(fileName)

  print("-----------------------------------------")
  print("--------------%d bit SUMMARY-------------" % BIT_NUM)
  print("-----------------------------------------")

  checkWithBase(fileName, result)

  print("---> timeToCreate: %fs" % timeToCreate)
  print("---> timeToSearch: %fs" % timeToSearch)
  print("---> memory:", memoryUsageMB, "MB")
  print("-----------------------------------------")
  print("------------%d bit SUMMARY END-----------" % BIT_NUM)
  print("-----------------------------------------")


