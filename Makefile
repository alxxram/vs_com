CC=g++
CFLAGS=-c -Wall

all: val_side

val_side: val_side.o vs_com.o
	$(CC) val_side.o vs_com.o -o val_side

val_side.o: val_side.cpp
	$(CC) $(CFLAGS) val_side.cpp

vs_com.o: vs_com.cpp
	$(CC) $(CFLAGS) vs_com.cpp

clean:
	rm -rf *o val_side

