
set -v

mkdir -p result
rm -f result/*

sudo python mininet/router_topo.py
sudo python mininet/router_topo_my.py

