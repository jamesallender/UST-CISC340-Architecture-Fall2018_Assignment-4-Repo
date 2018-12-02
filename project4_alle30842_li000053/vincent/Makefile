CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: simVincent.o
	$(CC) $(CFLAGS) simVincent.o -o simVincent $(LDFLAGS) -lm

simVincent.o: simVincent.c
	$(CC) $(CFLAGS) -c simVincent.c

clean:
	rm simVincent *.o *.*~ *~
