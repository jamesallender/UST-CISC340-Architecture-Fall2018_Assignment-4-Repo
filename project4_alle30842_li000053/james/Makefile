CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: simJames.o
	$(CC) $(CFLAGS) simJames.o -o simJames $(LDFLAGS) -lm

simJames.o: simJames.c
	$(CC) $(CFLAGS) -c simJames.c

clean:
	rm simJames *.o *.*~ *~
