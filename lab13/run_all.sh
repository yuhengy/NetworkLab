
set -v

mkdir -p result
rm -f result/*

sudo python mininet/nat_topo1.py
sudo python mininet/nat_topo2.py

