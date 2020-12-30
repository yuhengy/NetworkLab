
set -v

mkdir -p result
rm -f result/*

sudo python mininet/tcp_topo.py

