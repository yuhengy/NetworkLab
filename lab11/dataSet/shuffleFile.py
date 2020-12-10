import random
import sys


if __name__ == "__main__":
  fileName = sys.argv[1]

  with open(fileName, 'r') as f:
    lines = f.readlines()
    random.shuffle(lines)

  with open(fileName + ".shuffle", 'w') as f:
    for line in lines:
      f.write(line)


