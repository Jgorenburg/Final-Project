#ifndef SHARED_H
#define SHARED_H

#include <stdlib.h>
#include "linkedlist.h"

#define ML 10
#define MS 100
// global variables
extern struct LinkedList* joblist;

extern int MAX_LINE_SIZE;
extern int MAX_NUM_LINES;
extern char argArray[ML][MS];
extern int argc;

extern char *specialChars;


#endif

