
CPPFile = $(shell find src -name '*.cpp')
CPPFile += $(shell find src/messyOldFramework -name '*.c' -not -name 'main.c')
HFile = $(shell find src -name '*.h')
HFile += $(shell find src/messyOldFramework -name '*.h')

.PHONY: nat clean
nat: build/nat

build/nat: $(CPPFile) $(HFile)
	mkdir -p build
	g++ -fpermissive $(CPPFile) -o $@ -lpthread

clean:
	rm -rf build
