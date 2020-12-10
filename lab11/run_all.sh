
BIT_NUM_all=(1 2 4 8)

rm -f result/*
mkdir -p result plots

python src/basePrefixTree.py dataSet/forwarding-table.txt 600000 >> result/result-largeRouterTable.txt 2>&1

for BIT_NUM in ${BIT_NUM_all[@]}; do
  python src/multibitPrefixTree.py dataSet/forwarding-table.txt 600000 ${BIT_NUM} >> result/result-largeRouterTable.txt 2>&1
  python src/multibitCompressedPrefixTree.py dataSet/forwarding-table.txt 600000 ${BIT_NUM} >> result/result-largeRouterTable.txt 2>&1
done

python src/basePrefixTree.py dataSet/forwarding-table.txt.shuffle 60 >> result/result-smallRouterTable.txt 2>&1

for BIT_NUM in ${BIT_NUM_all[@]}; do
  python src/multibitPrefixTree.py dataSet/forwarding-table.txt.shuffle 60 ${BIT_NUM} >> result/result-smallRouterTable.txt 2>&1
  python src/multibitCompressedPrefixTree.py dataSet/forwarding-table.txt.shuffle 60 ${BIT_NUM} >> result/result-smallRouterTable.txt 2>&1
done

