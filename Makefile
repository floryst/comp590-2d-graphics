CC = g++ -g

CC_DEBUG = $(CC) 
CC_RELEASE = $(CC) -O3 -DNDEBUG

G_SRC = src/GBitmap.cpp src/GTime.cpp src/GPaint.cpp *.cpp

# need libpng to build
#
G_INC = -Iinclude -I/opt/local/include -L/opt/local/lib

all: test bench image

test : apps/test.cpp $(G_SRC)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/test.cpp -lpng -o test

bench : apps/bench.cpp $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/bench.cpp -lpng -o bench

image : apps/image.cpp $(G_SRC)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/image.cpp -lpng -o image

# needs xwindows to build
#
X_INC = -I/usr/X11R6/include -I/usr/X11R6/include/X11 -L/usr/X11R6/lib -L/usr/X11R6/lib/X11 -lX11

xapp: apps/xapp.cpp $(G_SRC) src/GXWindow.cpp include/GXWindow.h
	$(CC_RELEASE) $(X_INC) $(G_INC) $(G_SRC) apps/xapp.cpp src/GXWindow.cpp -lpng -o xapp


clean:
	rm -rf test bench image xapp *.dSYM

