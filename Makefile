CC=gcc
CFLAGS=-g -Wall -Wextra $(shell pkg-config --cflags sndfile)
LDFLAGS=$(shell pkg-config --static --libs sndfile)
OBJ=main.o

all: msproc

msproc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

main.o: main.c

clean:
	rm -f msproc *.o *~
