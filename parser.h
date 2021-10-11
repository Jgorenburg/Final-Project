#ifndef PARSER_H
#define PARSER_H

// parser program in C 
// that process command line input for the shell
// compile with "-lreadline"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shared.h"

#ifndef NULL
#define NULL (void *) 0
#endif
#define MAX_CMD MAX_NUM_LINES
#define MAX_CMD_LEN MAX_LINE_SIZE
#define SYMBOLS "&%;|><"

// global variables
//extern int argc;   // number of arguments
//extern char argArray[MAX_CMD][MAX_CMD_LEN];    // 2d array for command line input

int i, j;   // variable for iteration


void init_arg();
int parse(char* buf);
int parserMain();

#endif
