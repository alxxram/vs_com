CC=g++
CFLAGS=-c -Wall

all: vs_com

vs_com: vs_com.o
	$(CC) vs_com.o -o vs_com

vs_com.o: vs_com.cpp
	$(CC) $(CFLAGS) vs_com.cpp

clean:
	rm -rf *o vs_com

