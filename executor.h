#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "shared.h"

/*extern struct LinkedList jobList;
extern char argArray[10][100];
extern int argc;
*/

enum procLoc{BG, FG} commandLoc;

char *specialChars = "&%;|><"; 

void specChar(char c);
int isSpecChar(char c);
void runProg(char *args[]);
// int main();
