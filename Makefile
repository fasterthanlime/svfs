CC=clang
CFLAGS =`pkg-config fuse --cflags` -g -Wall
LDFLAGS=`pkg-config fuse --libs`

svfs : svfs.o
	${CC} ${CFLAGS} svfs.o -o svfs ${LDFLAGS} 

svfs.o : svfs.c params.h
	${CC} ${CFLAGS} -c svfs.c

clean:
	rm -f svfs *.o
