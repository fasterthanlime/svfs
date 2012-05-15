svfs : svfs.o
	gcc -g `pkg-config fuse --libs` -o svfs svfs.o

svfs.o : svfs.c params.h
	gcc -g -Wall `pkg-config fuse --cflags` -c svfs.c

clean:
	rm -f svfs *.o
