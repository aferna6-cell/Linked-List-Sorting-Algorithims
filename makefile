# Makefile for MP3
# Aidan Fernandes
# aferna6
# ECE 2230 Fall 2025

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target executable
TARGET = lab3

# Object files
OBJS = lab3.o llist.o ids_support.o

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile lab3.c
lab3.o: lab3.c datatypes.h llist.h ids_support.h
	$(CC) $(CFLAGS) -c lab3.c

# Compile llist.c
llist.o: llist.c datatypes.h llist.h
	$(CC) $(CFLAGS) -c llist.c

# Compile ids_support.c
ids_support.o: ids_support.c datatypes.h llist.h ids_support.h
	$(CC) $(CFLAGS) -c ids_support.c

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET) core a.out

# Design check - verify no illegal access to private variables
design:
	@grep -e "-> *ll_front" lab3.c ids_support.c ||:
	@grep -e "-> *ll_back" lab3.c ids_support.c ||:
	@grep -e "-> *ll_entry_count" lab3.c ids_support.c ||:
	@grep -e "-> *ll_sorted_state" lab3.c ids_support.c ||:
	@grep -e "-> *ll_next" lab3.c ids_support.c ||:
	@grep -e "-> *ll_prev" lab3.c ids_support.c ||:
	@grep -e "-> *data_ptr" lab3.c ids_support.c ||:
	@grep "compare_fun" lab3.c ids_support.c ||:
	@grep "llist_elem_t" lab3.c ids_support.c ||:
	@grep "generator_id" llist.c ||:
	@grep "dest_ip_addr" llist.c ||:
	@grep "alert_t" llist.c ||:
	@grep "ids_" llist.c ||:

.PHONY: all clean design
