CC = g++ -g

CC_DEBUG = @$(CC)
CC_RELEASE = @$(CC) -O3 -DNDEBUG

G_SRC = src/GContext_base.cpp src/GBitmap.cpp src/GTime.cpp src/GPaint.cpp *.cpp

# need libpng to build
#
G_INC = -Iinclude -Iapps -I/opt/local/include -L/opt/local/lib

all: test bench image

test : apps/test.cpp $(G_SRC)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/test.cpp -lpng -o test

bench : apps/bench.cpp $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/bench.cpp -lpng -o bench

image : apps/image.cpp $(G_SRC)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/image.cpp -lpng -o image

# needs xwindows to build
#
X_INC = -I/opt/X11/include -L/opt/X11/lib -I/usr/X11R6/include -I/usr/X11R6/include/X11 -L/usr/X11R6/lib -L/usr/X11R6/lib/X11

XAPP_SRC = apps/xapp.cpp src/GXWindow.cpp

xapp: $(XAPP_SRC) $(G_SRC)
	$(CC_DEBUG) $(X_INC) $(G_INC) $(G_SRC) $(XAPP_SRC) -lpng -lX11 -o xapp

SLIDE_SRC = apps/xslide.cpp src/GXWindow.cpp apps/GSlide.cpp apps/slide/slide_*.cpp

xslide: $(SLIDE_SRC) $(G_SRC)
	$(CC_DEBUG) $(X_INC) $(G_INC) $(G_SRC) $(SLIDE_SRC) -lpng -lX11 -o xslide

clean:
	@rm -rf test bench image xapp xslide *.dSYM

