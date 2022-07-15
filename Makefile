# Prototype for now
all : blockchain.out wallet.out
override CC	:= g++

CFLAGS		+= -lssl
CFLAGS		+= -lcrypto
CFLAGS		+= -lcryptopp
CFLAGS		+= -lstdc++fs
CFLAGS		+= -ltorrent-rasterbar
CFLAGS		+= --std=c++17
CFLAGS		+= -g
CFLAGS		+= -lcpr


SRC                           := $(shell find src -type f -name '*.cpp')
SRC_BLOCKCHAIN		      := apps/blockchain.cpp
SRC_WALLET		      := apps/wallet.cpp

blockchain.out : $(SRC_BLOCKCHAIN)
	$(CC) $(SRC_BLOCKCHAIN) $(SRC) $(CFLAGS) -o blockchain.out 

wallet.out : $(SRC_WALLET)
	$(CC) $(SRC_WALLET) $(SRC) $(CFLAGS) -o wallet.out

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)

run : blockchain.out
	./$<

clean : blockchain.out
	rm -R $^
