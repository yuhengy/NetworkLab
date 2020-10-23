mkdir -p result
make -C src clean
make -C src

sudo python three_nodes_bw.py
