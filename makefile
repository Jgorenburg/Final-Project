CC = gcc

CFLAGS = -g -Wall

all: shell readdisk format t

t: donothing.c
	$(CC) $(CFLAGS) -o t donothing.c

readdisk: diskReader.c format.h structs.o 
	$(CC) $(CFLAGS) -o diskread diskReader.c

fs.o: fs.c fs.h structs.o shared.o 
	$(CC) $(CFLAGS) -c fs.c

format: format.c format.h struct.o
	$(CC) $(CFLAGS) -o format format.c

shell: fs.o main.o shared.o parser.o linkedlist.o jobs.o executor.o
	$(CC) $(CFLAGS) -o shell main.o shared.o parser.o linkedlist.o jobs.o executor.o fs.o -lreadline

main.o: main.c main.h parser.o shared.o fs.o
	$(CC) $(CFLAGS) -c main.c

shared.o: shared.c shared.h struct.o 
	$(CC) $(CFLAGS) -c shared.c

struct.o: structs.c structs.h
	$(CC) $(CFLAGS) -c structs.c

parser.o: parser.c parser.h shared.o fs.o 
	$(CC) $(CFLAGS) -c parser.c -lreadline

linkedlist.o: linkedlist.c linkedlist.h jobs.o
	$(CC) $(CFLAGS)	-c linkedlist.c

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c

executor.o: executor.c executor.h fs.o shared.o 
	$(CC) $(CFLAGS) -c executor.c

clean: 
	$(RM) shell *.o *~
