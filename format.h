#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "fs/structs.h"

#define DEFAULTSIZE 1000000
#define BLOCKSIZE 512
#define SBSIZE 512
#define OFFSET (SBSIZE * 2 + BLOCKSIZE)
#define IOFFSET 0
#define ENDLIST -1


int main(int argc, char *argv[]);
