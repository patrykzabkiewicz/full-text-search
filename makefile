CC=g++-14 
CFLAGS=-std=c++23 -Wall

all:
	$(CC) $(CFLAGS) full-text-search.cpp -o full-text-search
