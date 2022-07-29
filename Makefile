# Prototype for now

all : wallet.out blockchain.out
override CC		      := g++
CFLAGS			      += -lssl
CFLAGS			      += -lcrypto
CFLAGS			      += -lcryptopp
CFLAGS			      += -lstdc++fs
CFLAGS			      += --std=c++17
CFLAGS		              += -g
CFLAGS			      += -lcpr

SRC                           := $(wildcard src/*.cpp)
SRC_BLOCKCHAIN		      := apps/blockchain.cpp
SRC_WALLET		      := apps/wallet.cpp


blockchain.out: $(SRC) $(SRC_BLOCKCHAIN)
	$(CC) $(SRC_BLOCKCHAIN) build/* $(CFLAGS) -o blockchain.out 

wallet.out: $(SRC) $(SRC_WALLET)
	$(CC) $(SRC_WALLET) build/* $(CFLAGS) -o wallet.out

run : blockchain.out
	./$<

move : objects
	mv $(wildcard *.o) build

objects : $(SRC)
	$(CC) -c $(SRC) $(CFLAGS)

wallet : wallet.out

blockchain : blockchain.out

clean:
ifneq (,$(wildcard blockchain.out))
	rm -R blockchain.out
	rm -R build/*
else ifneq (,$(wildcard wallet.out))
	rm -R wallet.out
	rm -R build/*
endif

.PHONY: clean
