CC = gcc
CFLAGS = -g -Wall -DDEBUG -pedantic -std=c99 -N
LDFLAGS = -levent -levent_pthreads -lao -lspotify -ljansson -lslutils

all: server

server: main.o server.o spotify.o player.o queue.o play_queue.o commands.o

.PHONY: clean
clean:
	rm -f server src/*.o
	rm -rf tmp
	rm -rf *.dSYM
