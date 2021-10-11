#ifndef CUTOR_H
#define CUTOR_H

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "shared.h"

#define MAX_CHARS_PER_LINE MAX_LINE_SIZE

/*extern struct LinkedList jobList;
extern char argArray[10][100];
extern int argc;
*/



enum procLoc{BG, FG} commandLoc;

//char *specialChars[10]; 

void specChar(char c);
int isSpecChar(char c);
bool builtIn(char* c, int args);
void runProg(char *args[]);
void execute();

#endif
