override CC 	:= g++

CFLAGS		+= -lcryptopp
CFLAGS		+= --std=c++17

SRC 		:= $(shell find . ../broadcast ../block ../rsa -type f -name '*.cpp')


wallet.out : $(SRC)
	$(CC) $(SRC) $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)

run : wallet.out
	./$<

clean : wallet.out
	rm -R $^
