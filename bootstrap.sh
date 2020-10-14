#!/usr/bin/env bash

echo "Acquire::http::proxy \"http://192.168.3.2:7890/\";" > /etc/apt/apt.conf.d/30proxy
apt-get update
add-apt-repository ppa:apt-fast/stable
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -y install apt-fast
sudo sh -c 'echo debconf apt-fast/maxdownloads string 32 | debconf-set-selections'
sudo sh -c 'echo debconf apt-fast/dlflag boolean true | debconf-set-selections'
sudo sh -c 'echo debconf apt-fast/aptmanager string apt-get | debconf-set-selections'


DEBIAN_FRONTEND=noninteractive apt-fast -y install wireshark tshark
apt-fast -y install mininet xterm python3-pip
apt-fast -y install xauth # For x11
pip3 install pysocks
#pip3 install numpy matplotlib --proxy='http://192.168.3.2:7890'
pip3 install numpy matplotlib --proxy='socks5://192.168.3.2:7890'


echo "export https_proxy=http://192.168.3.2:7890 http_proxy=http://192.168.3.2:7890 all_proxy=socks5://192.168.3.2:7890" >> /home/vagrant/.bashrc

## one sudo will last until log out
sudo sed -i 's/env_reset/env_reset,timestamp_timeout=-1/g' /etc/sudoers