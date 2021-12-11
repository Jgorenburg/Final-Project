#include "shared.h"
#include "jobs.h"

// global variables
struct LinkedList* joblist;

FILE* disk;

int MAX_LINE_SIZE = MS;
int MAX_NUM_LINES = ML;
char argArray[ML][MS];
int argc = 0;

char *specialChars = "&%;|><";
