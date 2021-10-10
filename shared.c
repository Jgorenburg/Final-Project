#include "shared.h"
#include "jobs.h"

// global variables
<<<<<<< HEAD
extern struct LinkedList jobList;
=======
struct LinkedList* joblist;

>>>>>>> b18c53ca69dc339be42905d316e62cf3a765bef5


// TODO use defines in parser
char argArray[10][100];
int argc = 0;

char *specialChars = "&%;|><";
