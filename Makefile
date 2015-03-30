CC=gcc
RUN=./
CFLAGS=-g
SFLAGS=-pthread -g
BIN=bin
SRC=src
COUT=c
SOUT=s

all: server client
	$(CC) $(SFLAGS) $(BIN)/server.o -o $(SOUT)
	$(CC) $(CFLAGS) $(BIN)/client.o -o $(COUT)

clean:
	@rm -rf $(BIN)
	@rm -rf $(SOUT)
	@rm -rf $(COUT)

init:
	@mkdir -p $(BIN)

server: init
	$(CC) -c $(CFLAGS) $(SRC)/server.c -o $(BIN)/server.o

client: init
	$(CC) -c $(CFLAGS) $(SRC)/client.c -o $(BIN)/client.o


