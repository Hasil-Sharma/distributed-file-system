#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lcrypto -lssl
OBJS = 
CFLAGS = -g -Wunused
all: dfs dfc

dfs:
	$(CC) -o bin/dfs -Iheaders headers/*.c dfs.c $(CFLAGS) $(LIBS)

dfc:
	$(CC) -o bin/dfc -Iheaders headers/*.c dfc.c $(CFLAGS) $(LIBS)

clean:
	rm -rf bin
	mkdir -p bin
