CC = gcc

CFLAGS = -g -Wall

all: shell format readdisk t

t: donothing.c
	$(CC) $(CFLAGS) -o t donothing.c

readdisk: diskReader.c format.h fs/structs.h 
	$(CC) $(CFLAGS) -o diskread diskReader.c

format: format.c format.h
	$(CC) $(CFLAGS) -o format format.c

shell: main.o shared.o parser.o linkedlist.o jobs.o executor.o
	$(CC) $(CFLAGS) -o shell main.o shared.o parser.o linkedlist.o jobs.o executor.o -lreadline

main.o: main.c main.h parser.o
	$(CC) $(CFLAGS) -c main.c

shared.o: shared.c shared.h
	$(CC) $(CFLAGS) -c shared.c

parser.o: parser.c parser.h shared.h 
	$(CC) $(CFLAGS) -c parser.c -lreadline

linkedlist.o: linkedlist.c linkedlist.h jobs.h
	$(CC) $(CFLAGS)	-c linkedlist.c

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c

executor.o: executor.c executor.h
	$(CC) $(CFLAGS) -c executor.c

clean: 
	$(RM) shell *.o *~
