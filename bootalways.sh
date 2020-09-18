
sudo sh -c "echo \"Acquire::http::proxy \\\"http://192.168.3.2:7890/\\\";\" > /etc/apt/apt.conf.d/30proxy"
#sudo sh -c "echo \"Acquire::http::proxy \\\"http://10.200.175.54:7890/\\\";\" > /etc/apt/apt.conf.d/30proxy"