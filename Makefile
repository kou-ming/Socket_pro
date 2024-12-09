CXX = gcc
PKG_CONFIG = pkg-config
GTK_CFLAGS = $(shell $(PKG_CONFIG) --cflags gtk4)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk4)
GTK_PATH = $(shell $(pkg-config --cflags --libs gtk4))

all: server client

server: server.c
	$(CXX) -o server server.c

client: client.c
	$(CXX) -o client client.c $(GTK_CFLAGS) $(GTK_LIBS)

gtkex: gtkex.c
	$(CXX) -o gtkex gtkex.c $(GTK_CFLAGS) $(GTK_LIBS)

clean:
	rm -f server
	rm -f client
	rm -f gtkex
