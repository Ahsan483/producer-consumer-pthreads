CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: prodcons

prodcons: prodcons.c
	$(CC) $(CFLAGS) -pthread -o $@ $<

clean:
	rm -f prodcons
