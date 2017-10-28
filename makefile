#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS =
OBJS = 
CFLAGS = -g
all: dfs dfc

dfs:
	$(CC) -o bin/dfs -Iheaders headers/*.c dfs.c $(CFLAGS) $(LIBS)

dfc:
	$(CC) -o bin/dfc -Iheaders headers/*.c dfc.c $(CFLAGS) $(LIBS)

clean:
	rm -rf bin
	mkdir -p bin
