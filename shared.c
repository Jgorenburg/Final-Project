#include "shared.h"
#include "jobs.h"

// global variables
struct LinkedList* joblist;



// TODO use defines in parser
char argArray[10][100];
int argc = 0;

char *specialChars = "&%;|><";
