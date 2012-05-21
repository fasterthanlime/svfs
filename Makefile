#CC=clang
CC=gcc
CFLAGS =`pkg-config fuse --cflags` -g -Wall -std=gnu99
LDFLAGS=`pkg-config fuse --libs`

svfs : svfs.o backup.o
	${CC} ${CFLAGS} svfs.o backup.o -o svfs ${LDFLAGS} 

svfs.o : svfs.c params.h
	${CC} ${CFLAGS} -c svfs.c

backup.o : backup.c backup.h
	${CC} ${CFLAGS} -c backup.c

clean:
	rm -f svfs *.o
