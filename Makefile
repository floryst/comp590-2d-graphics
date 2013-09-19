CC = g++

CC_DEBUG = $(CC) -g
CC_RELEASE = $(CC) -O3 -DNDEBUG

all: test bench image

test : test.cpp GContext2.cpp
	$(CC_DEBUG) test.cpp GContext2.cpp -o test

bench : bench.cpp GTime.cpp GContext2.cpp
	$(CC_RELEASE) bench.cpp GTime.cpp GContext2.cpp -o bench

image : image.cpp GBitmap.cpp GContext2.cpp
	$(CC_DEBUG) image.cpp GBitmap.cpp GContext2.cpp -lpng -o image

clean:
	rm -rf test bench image *.dSYM

