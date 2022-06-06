# Prototype for now
override CC	:= g++

CFLAGS		+= -lssl
CFLAGS		+= -lcrypto
CFLAGS		+= -lstdc++fs
CFLAGS		+= -ltorrent-rasterbar
CFLAGS		+= --std=c++17
CFLAGS		+= -g
CFLAGS		+= -lcpr

SRC                           := $(shell find . -type f -name '*.cpp')

blockchain.out : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o blockchain.out 

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)


run : blockchain.out
	./$<

clean : blockchain.out
	rm -R $<
