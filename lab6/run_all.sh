mkdir -p result
rm -f result/*

make -C src clean
make -C src

sudo python mininet/four_node_ring.py
bash mininet/dump_output.sh 4 > result/PART1-dump_output.txt

sudo python mininet/seven_node.py
bash mininet/dump_output.sh 7 > result/PART2-dump_output.txt
