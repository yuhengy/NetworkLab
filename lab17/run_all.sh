
set -v

mkdir -p result
rm -f result/*

make

rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_myH1Server-myH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_myH1Server-refH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_refH1Server-myH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_refH1Server-refH2Client-BIGFILE.py

