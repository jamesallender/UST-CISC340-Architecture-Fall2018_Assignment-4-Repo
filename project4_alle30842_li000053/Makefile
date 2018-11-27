CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: sim.o
	$(CC) $(CFLAGS) sim.o -o sim $(LDFLAGS)

sim.o: sim.c
	$(CC) $(CFLAGS) -c sim.c

clean:
	rm sim *.o *.*~ *~
