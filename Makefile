CC = gcc
CFLAGS = -g -Wall -DDEBUG -pedantic -std=c99
LDFLAGS = -lpthread -levent -levent_pthreads -lao -lspotify -ljansson

TARGET   = server
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

OBJECTS := main.o commands.o play_queue.o player.o queue.o server.o spotify.o arraylist.o playlist.o
OBJECTS := $(addprefix $(OBJDIR)/,$(OBJECTS))

.PHONY: all
all: bin/server

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "Linking complete!"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	rm -f obj/*.o
	rm -f bin/server
	rm -rf tmp
	rm -rf *.dSYM
