all: jobs parser

jobs: jobs.c
	gcc -g -Wall -o jobs jobs.c -I.

parser: parser.c
	gcc -g -Wall -o parser parser.c -lreadline
