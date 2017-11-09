#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lcrypto -lssl
OBJS = 
CFLAGS = -g -Wunused
all: clean dfs dfc start run

dfs:
	$(CC) -o bin/dfs -Iheaders headers/*.c dfs.c $(CFLAGS) $(LIBS)

dfc:
	$(CC) -o bin/dfc -Iheaders headers/*.c dfc.c $(CFLAGS) $(LIBS)

kill:
	fuser -k {10001,10002,10003,10004}/tcp

clear:
	rm -rf DFS*/*

clean:
	rm -rf bin
	mkdir -p bin

start:
	bin/dfs /DFS1 10001 &> logs/dfs1.log &
	bin/dfs /DFS2 10002 &> logs/dfs2.log &
	bin/dfs /DFS3 10003 &> logs/dfs3.log &
	bin/dfs /DFS4 10004 &> logs/dfs4.log &
	
run:
	tail -f logs/dfs*

dc:
	rm bin/dfc
	$(CC) -o bin/dfc -Iheaders headers/*.c dfc.c $(CFLAGS) $(LIBS)
	gdb -tui bin/dfc --args bin/dfc conf/dfc.conf 2> logs/client.log

ds:
	rm bin/dfs
	$(CC) -o bin/dfs -Iheaders headers/*.c dfs.c $(CFLAGS) $(LIBS)
	gdb -tui bin/dfs --args bin/dfs /DFS1 10001

client:
	rm bin/dfc
	$(CC) -o bin/dfc -Iheaders headers/*.c dfc.c $(CFLAGS) $(LIBS)
	bin/dfc conf/dfc.conf

