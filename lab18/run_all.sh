
set -v

mkdir -p result
rm -f result/*
rm cwndLog.json
rm howCwndChange.png

make

rm -f mininet/server-output.dat
sudo python mininet/tcp_topo_myH1Server-myH2Client-BIGFILE.py

python3 scripts/plotCwnd.py cwndLog.json
