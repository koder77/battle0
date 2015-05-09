
CC = gcc
AR = tar
GZ = gzip

CFLAGS= -O2
LDFLAGS = -lSDLmain -lSDL -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_net -lm


all: project


# Compiler:
project:
	$(CC) -o battle0-client $(CFLAGS) battle0-client.c network.c data-exchange.c getpixel.c graphics.c input.c units.c $(LDFLAGS)
	$(CC) -o battle0-server $(CFLAGS) battle0-server.c network.c data-exchange.c getpixel.c $(LDFLAGS)

clean:
	rm -f *.o
	rm -f *.exe

#Distribution
dist:
	rm -f *.o
	rm -f *.exe
	$(AR) --create --file=battle0.tar *.c *.h Makefile
	$(GZ) -c battle0.tar >battle0.tar.gz
	rm -f battle0.tar
