
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


class multibitCompressedPrefixTree_c():
  def __init__(self):
      self.root = {"match": False, "skip": []}
      self.skipedNodeNum = 0
      self.originalNodeNum = 0

  def insertEntry(self, IPmultibit, IPStr, mask, iface):
    maskPadder = (32-mask) % BIT_NUM
    node  = self.root
    insertLog = []
    for multibit, _ in zip(IPmultibit, range(int(mask/BIT_NUM))):
      #if "IPStr" in node.keys():
      #  insertLog.append([multibit, node["match"], node["skip"], node["IPStr"], node["mask"], node["iface"]])
      #else:
      #  insertLog.append([multibit, node["match"], node["skip"]])
      node = node.setdefault(multibit, {"match": False, "skip": []})
    
    if maskPadder == 0:
      nodeList = [node]
    else:
      multibitRemainer = (IPmultibit[int(mask/BIT_NUM)] \
                         >> maskPadder) \
                         << maskPadder
      nodeList = []
      for i in range(2**maskPadder):
        nodeList.append(node.setdefault(multibitRemainer+i, {"match": False, "skip": []}))

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

  def compressTree(self):
    self.root = self.compressRecursive(self.root)

  def compressRecursive(self, node):
    childrenList = [i for i in node.keys() if i not in ["match", "skip", "IPStr", "mask", "iface"]]
    
    if len(childrenList) == 1 and (not node[childrenList[0]]["match"]):
      for key in [i for i in node.keys() if i not in childrenList]:
        node[childrenList[0]][key] = node[key]
      node[childrenList[0]]["skip"].append(childrenList[0])
      node = node[childrenList[0]]
      childrenList = [i for i in node.keys() if i not in ["match", "skip", "IPStr", "mask", "iface"]]
      self.skipedNodeNum += 1
      self.originalNodeNum += 1
      node = self.compressRecursive(node)
    else:
      for child in childrenList:
        self.originalNodeNum += len(childrenList)
        node[child] = self.compressRecursive(node[child])

    return node




  def searchIP(self, IPmultibit, IPStr):
    node = self.root
    lastMatchNode = {"match": False, "skip": 0, "IPStr": "0", "mask": 0, "iface": 0}

    searchLog = []
    i = 0
    while i < 32 / BIT_NUM:
      canSkip = True
      for matchMultibit in node["skip"]:
        if IPmultibit[i] != matchMultibit:
          canSkip = False
          break
        i += 1
      if not canSkip:
        break

      multibit = IPmultibit[i]
      #if "IPStr" in node.keys():
      #  searchLog.append([multibit, node["match"], node["skip"], node["IPStr"], node["mask"], node["iface"]])
      #else:
      #  searchLog.append([multibit, node["match"], node["skip"]])
      if multibit not in node:
        break
      node = node[multibit]
      if node["match"] == True:
        lastMatchNode = node
      i += 1

    #if True:
    #  print("----------------searchLog----------------")
    #  for searchLog_one in searchLog:
    #    print(searchLog_one)
    #  print("----------------searchLog----------------")

    return lastMatchNode["match"], lastMatchNode["IPStr"], \
           lastMatchNode["mask"] , lastMatchNode["iface"], searchLog

def generateResult(fileName, routerSize):

  dataSet = changeToMultibit(parseDataSet(fileName))
  result = []

  timeStart = time.time()
  multibitCompressedPrefixTree = multibitCompressedPrefixTree_c()
  for entry in dataSet[:routerSize]:
    multibitCompressedPrefixTree.insertEntry( \
      entry["IPmultibit"], entry["IPStr"], entry["mask"], entry["iface"] \
    )
  #print(multibitCompressedPrefixTree.root)
  #print("-------------------------")
  multibitCompressedPrefixTree.compressTree()
  timeToCreate = time.time() - timeStart

  #print(multibitCompressedPrefixTree.root)
  memoryUsageMB = int(int((str(hpy().heap())).split()[10]) / 1024 / 1024)

  timeStart = time.time()
  for i, entry in enumerate(dataSet):
    match, IPStr, mask, iface, searchLog = \
      multibitCompressedPrefixTree.searchIP(entry["IPmultibit"], entry["IPStr"])
    result.append({"match": match, "IPStr": IPStr, "mask": int(mask), "iface": int(iface)})

    #if not match:
    #  print("Cannot find %d-th entry: " % (i + 1), entry)
    #  print("----------------searchLog----------------")
    #  for searchLog_one in searchLog:
    #    print(searchLog_one)
    #  print("----------------searchLog----------------")
    #  assert(False)

    #print("From ", entry, " --- Find:", match, IPStr, mask, iface)
  timeToSearch = time.time() - timeStart

  return result, timeToCreate, timeToSearch, \
    multibitCompressedPrefixTree.skipedNodeNum/multibitCompressedPrefixTree.originalNodeNum, memoryUsageMB





if __name__ == "__main__":
  fileName = sys.argv[1]
  routerSize = int(sys.argv[2])
  BIT_NUM = int(sys.argv[3])

  result, timeToCreate, timeToSearch, skipedNodeRate, memoryUsageMB \
    = generateResult(fileName, routerSize)

  print("\n")
  print("----------------------------------------------------")
  print("--------------%d bit Compressed SUMMARY-------------" % BIT_NUM)
  print("----------------------------------------------------")

  checkWithBase(fileName, routerSize, result)

  print("---> timeToCreate: %fs" % timeToCreate)
  print("---> timeToSearch: %fs" % timeToSearch)
  print("---> skipedNodeRate: %f" % skipedNodeRate)
  print("---> memory:", memoryUsageMB, "MB")
  print("----------------------------------------------------")
  print("------------%d bit Compressed SUMMARY END-----------" % BIT_NUM)
  print("----------------------------------------------------")


