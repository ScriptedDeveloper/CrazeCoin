# Prototype for now
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))
all: blockchain.out wallet.out
override CC		      := g++
CFLAGS			      += -lssl
CFLAGS			      += -lcryptopp
CFLAGS			      += -lstdc++fs
CFLAGS			      += --std=c++17
CFLAGS		              += -g
CFLAGS			      += -lcpr

SRC_BLOCKCHAIN		      := apps/blockchain.cpp
SRC_WALLET		      := apps/wallet.cpp

blockchain.out: $(SRC_BLOCKCHAIN) $(OBJ)
	$(CC) apps/blockchain.cpp build/* $(CFLAGS) -o blockchain.out 

wallet.out: $(SRC_WALLET) $(OBJ)
	$(CC) apps/wallet.cpp build/* $(CFLAGS) -o wallet.out

run : blockchain.out
	./$<

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -R *.out
	rm -R build/*

.PHONY: clean

