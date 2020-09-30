
## If you need an enviornment
#  Just use this instruction
#  Note, also uncomment the `"` in the last line

# vagrant up && vagrant ssh -- "

rm -rf result
make clean
make all
sudo python src/luanchMininet.py
## The results of these three tests are generated in ./results
#  TEST1 myClient to myServer
#  TEST2 wgetClient to myServer
#  TEST3 myClient to pythonServer

# "
