#ifndef SHARED_H
#define SHARED_H

#include "linkedlist.h"

// global variables
extern struct LinkedList* joblist;

// TODO use defines in parser
extern char argArray[10][100];
extern int argc;

extern char *specialChars;
#endif
