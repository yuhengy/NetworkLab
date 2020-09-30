
## If you need an enviornment
#  Just use this instruction
#  Note, also uncomment the `"` in the last line

# vagrant up && vagrant ssh -- "

rm -rf result
make clean
make all
python src/luanchMininet.py

# "
