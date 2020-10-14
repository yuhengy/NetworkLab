
mkdir -p build result
make

## STEP1 test ping
## STEP2 iperf performance
sudo python mininet/three_nodes_bw.py 

## STEP3 test hub loop
sudo python mininet/hub_loop.py
