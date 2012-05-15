CFLAGS =`pkg-config fuse --cflags` -g -Wall
LDFLAGS=`pkg-config fuse --libs`

svfs : svfs.o
	gcc ${CFLAGS} svfs.o -o svfs ${LDFLAGS} 

svfs.o : svfs.c params.h
	gcc ${CFLAGS} -c svfs.c

clean:
	rm -f svfs *.o
