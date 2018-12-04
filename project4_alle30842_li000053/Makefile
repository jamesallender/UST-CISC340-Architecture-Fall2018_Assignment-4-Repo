CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: cacheSim.o
	$(CC) $(CFLAGS) cacheSim.o -o cacheSim $(LDFLAGS) -lm

cacheSim.o: cacheSim.c
	$(CC) $(CFLAGS) -c cacheSim.c

clean:
	rm cacheSim *.o *.*~ *~
