#ifndef SHARED_H
#define SHARED_H

#include <stdlib.h>
#include "linkedlist.h"
#include "structs.h"

#define ML 10
#define MS 103
// global variables
extern struct LinkedList* joblist;

extern FILE* disk;
extern struct diskimage* dimage; 

extern int MAX_LINE_SIZE;
extern int MAX_NUM_LINES;
extern char argArray[ML][MS];
extern int argc;
extern int curDir;

extern char *specialChars;


#endif

