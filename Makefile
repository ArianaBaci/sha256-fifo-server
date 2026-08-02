CC=gcc
OPENSSL_PREFIX=$(shell brew --prefix openssl@3)

CFLAGS=-Wall -std=gnu99 -Wno-deprecated-declarations
INCLUDES=-I./inc -I$(OPENSSL_PREFIX)/include
LDFLAGS=-L$(OPENSSL_PREFIX)/lib
LDLIBS=-lssl -lcrypto -lpthread

SERVER_SRCS=src/errExit.c src/digest.c src/threadpool.c src/cache.c src/server.c
CLIENT_SRCS=src/errExit.c src/client.c

SERVER_OBJS=$(SERVER_SRCS:.c=.o)
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)

all: server client

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

client: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

.c.o:
	@echo "Compiling: "$
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	@rm -f src/*.o client server
	@echo "Removed object files and executables..."