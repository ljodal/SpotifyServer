CC = gcc
CFLAGS = -g -Wall -DDEBUG -pedantic -std=c99 -N
LDFLAGS = -levent -levent_pthreads -lao -lspotify -ljansson

all: server

server: main.o server.o spotify.o player.o queue.o play_queue.o commands.o arraylist.o

.PHONY: clean
clean:
	rm -f server *.o
	rm -rf tmp
	rm -rf *.dSYM
