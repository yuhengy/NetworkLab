
set -v

mkdir -p result
rm -f result/*

make

sudo python mininet/tcp_topo_myH1Server-myH2Client.py
sudo python mininet/tcp_topo_myH1Server-refH2Client.py
sudo python mininet/tcp_topo_refH1Server-myH2Client.py

