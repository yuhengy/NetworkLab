
CLIENTCPPFile = $(shell find src -name '*.cpp' -not -name 'MAINServer.cpp')
SERVERCPPFile = $(shell find src -name '*.cpp' -not -name 'MAINClient.cpp')
HFile = $(shell find src -name '*.h')

.PHONY: all client server clean
all: build/client build/server
client: build/client
server: build/server
clean:
	rm -rf build

build/client: $(CLIENTCPPFile) $(HFile)
	mkdir -p build
	g++ $(CLIENTCPPFile) -o $@
build/server: $(SERVERCPPFile) $(HFile)
	mkdir -p build
	g++ $(SERVERCPPFile) -o $@ -lpthread