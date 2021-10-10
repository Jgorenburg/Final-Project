CC = gcc

CFLAGS = -g -Wall

all: shell

shell: shared.o parser.o linkedlist.o jobs.o executor.o
	$(CC) $(CFLAGS) -o shell shared.o parser.o linkedlist.o jobs.o executor.o -lreadline

shared.o: shared.c shared.h
	$(CC) $(CFLAGS) -c shared.c

parser.o: parser.c shared.h 
	$(CC) $(CFLAGS) -c parser.c -lreadline

linkedlist.o: linkedlist.c linkedlist.h jobs.h
	$(CC) $(CFLAGS)	-c linkedlist.c

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c

executor.o: executor.c executor.h linkedlist.h jobs.h shared.h
	$(CC) $(CFLAGS) -c executor.c

clean: 
	$(RM) shell *.o *~
