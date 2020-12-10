
import sys
from guppy import hpy
import time

from parseDataSet import parseDataSet


class basePrefixTree_c():
  def __init__(self):
      self.root = {"match": False}


  def insertEntry(self, IPStr, mask, iface):
    node  = self.root
    for bit, _ in zip(IPStr, range(mask)):
      node = node.setdefault(bit, {"match": False})
    node["match"] = True
    node["IPStr"] = IPStr
    node["mask"]  = mask
    node["iface"] = iface


  def searchIP(self, IPStr):
    node = self.root
    lastMatchNode = {"match": False, "IPStr": "0", "mask": 0, "iface": 0}
    for bit in IPStr:
      if bit not in node:
        break
      node = node[bit]
      if node["match"] == True:
        lastMatchNode = node

    return lastMatchNode["match"], lastMatchNode["IPStr"], \
           lastMatchNode["mask"] , lastMatchNode["iface"]

def generateBaseResult(fileName, routerSize):

  dataSet = parseDataSet(fileName)
  baseResult = []

  timeStart = time.time()
  basePrefixTree = basePrefixTree_c()
  for entry in dataSet[:routerSize]:
    basePrefixTree.insertEntry(entry["IPStr"], entry["mask"], entry["iface"])
  timeToCreate = time.time() - timeStart

  #print(basePrefixTree.root)
  memoryUsageMB = int(int((str(hpy().heap())).split()[10]) / 1024 / 1024)

  timeStart = time.time()
  for i, entry in enumerate(dataSet):
    match, IPStr, mask, iface = basePrefixTree.searchIP(entry["IPStr"])
    baseResult.append({"match": match, "IPStr": IPStr, "mask": int(mask), "iface": int(iface)})

    #if not match:
    #  print("Cannot find %d-th entry: " % (i + 1), entry)
    #  assert(False)

    #print("From ", entry, " --- Find:", match, IPStr, mask, iface)
  timeToSearch = time.time() - timeStart

  return baseResult, timeToCreate, timeToSearch, memoryUsageMB

def checkWithBase(fileName, routerSize, result):

  baseResult, _, _, _ = generateBaseResult(fileName, routerSize)
  assert(len(baseResult) == len(result) and "Error: result Len Mismatch.")
  for i, (baseEntry, entry) in enumerate(zip(baseResult, result)):
    for baseValue, value in zip(baseEntry.values(), entry.values()):
      if baseValue != value:
        print("Error: result Value Mismatch at %d-th entry." % (i + 1))
        print("Right result: ", baseEntry)
        print("But our result: ", entry)
        assert(False)

  print("---> check result pass")




if __name__ == "__main__":
  fileName = sys.argv[1]
  routerSize = int(sys.argv[2])

  baseResult, timeToCreate, timeToSearch, memoryUsageMB = generateBaseResult(fileName, routerSize)

  print("\n")
  print("------------------------------------------")
  print("---------------BASE SUMMARY---------------")
  print("------------------------------------------")
  print("---> timeToCreate: %fs" % timeToCreate)
  print("---> timeToSearch: %fs" % timeToSearch)
  print("---> memory:", memoryUsageMB, "MB")
  print("------------------------------------------")
  print("-------------BASE SUMMARY END-------------")
  print("------------------------------------------")


