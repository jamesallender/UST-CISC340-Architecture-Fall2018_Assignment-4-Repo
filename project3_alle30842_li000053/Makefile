CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: pipelineSimulator.o
	$(CC) $(CFLAGS) pipelineSimulator.o -o pipelineSimulator $(LDFLAGS)

pipelineSimulator.o: pipelineSimulator.c
	$(CC) $(CFLAGS) -c pipelineSimulator.c

clean:
	rm pipelineSimulator *.o *.*~ *~
