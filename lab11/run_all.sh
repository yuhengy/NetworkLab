
BIT_NUM_all=(1 2 4 8 16 32)

python src/basePrefixTree.py dataSet/forwarding-table.txt

for BIT_NUM in ${BIT_NUM_all[@]}; do
  python src/optimizedPrefixTree.py dataSet/forwarding-table.txt ${BIT_NUM}
done
