CC=gcc
RUN=./
CFLAGS=-g
SFLAGS=-l sqlite3 -pthread
DFLAGS=-l sqlite3
BIN=bin
SRC=src
COUT=c
SOUT=s
DOUT=db

all: database server client

clean:
	@rm -rf $(SOUT)
	@rm -rf $(COUT)
	@rm -rf $(DOUT)

server:
	$(CC) $(SRC)/server.c $(SFLAGS) -o $(SOUT)

client:
	$(CC) $(SRC)/client.c $(CFLAGS) -o $(COUT)

database:
	$(CC) $(SRC)/createDB.c $(DFLAGS) -o $(DOUT)