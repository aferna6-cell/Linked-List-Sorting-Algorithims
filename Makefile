# Makefile for MP3
# ECE 2230 Fall 2025

CC = gcc
CFLAGS = -Wall -g
VALGRIND = valgrind --leak-check=yes

# Main program for MP3
lab3: lab3.o ids_support.o llist.o
	$(CC) $(CFLAGS) -o lab3 lab3.o ids_support.o llist.o

lab3.o: lab3.c llist.h ids_support.h datatypes.h
	$(CC) $(CFLAGS) -c lab3.c

ids_support.o: ids_support.c ids_support.h llist.h datatypes.h
	$(CC) $(CFLAGS) -c ids_support.c

llist.o: llist.c llist.h datatypes.h
	$(CC) $(CFLAGS) -c llist.c

# Helper to build geninput if present
geninput: geninput.c
	$(CC) $(CFLAGS) -o geninput geninput.c

# Test wrappers (scripts provided by instructor)
test: lab3 geninput
	./run.sh

valtest: lab3 geninput
	./valrun.sh

longrun: lab3 geninput
	./longrun.sh

# Clean up generated files
clean:
	rm -f *.o lab3 geninput
	rm -f gradingout_* out_* core

.PHONY: test valtest longrun clean
