
## If you need an enviornment
#  Just use this instruction
#  Note, also uncomment the `"` in the last line

 vagrant up && vagrant ssh -- " cd /NetworkLab/lab3

rm -rf result
make clean
make all
sudo python src/launchMininet.py
## The results of these three tests are generated in ./results
#  TEST1 myClient to myServer
#      a. continuous request
#      b. parallel request, to check the correctness, all requests finish at the same time.
#      c. File not found
#  TEST2 wgetClient to myServer
#      a. continuous request
#      b. parallel request, to check the correctness, all requests finish at the same time.
#      c. File not found
#  TEST3 myClient to pythonServer
#      a. continuous request
#      c. File not found

 "
