
set -v

mkdir -p result
#rm -f result/*

make

# to run following commands, you need change `TCPApp.cpp`.
#sudo python mininet/tcp_topo_myH1Server-myH2Client.py
#sudo python mininet/tcp_topo_myH1Server-refH2Client.py
#sudo python mininet/tcp_topo_refH1Server-myH2Client.py
#sudo python mininet/tcp_topo_refH1Server-refH2Client.py


rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_myH1Server-myH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_myH1Server-refH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_refH1Server-myH2Client-BIGFILE.py
rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_refH1Server-refH2Client-BIGFILE.py

