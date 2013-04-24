CC = gcc
CFLAGS = -g -Wall -DDEBUG -pedantic -std=c99 -N
LDFLAGS = -levent -levent_pthreads -lao -lspotify -lslutils

all: server

server: src/main.o src/server.o src/spotify.o src/player.o src/queue.o src/play_queue.o src/commands.o

.PHONY: clean
clean:
	rm -f server src/*.o
	rm -rf tmp
	rm -rf *.dSYM
