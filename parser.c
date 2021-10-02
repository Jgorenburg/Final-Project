// parser program in C 
// that process command line input for the shell
// compile with "-lreadline"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#ifndef NULL
#define NULL (void *) 0
#endif
#define MAX_CMD 10
#define MAX_CMD_LEN 100

// global variables
int argc;   // number of arguments
char argArray[MAX_CMD][MAX_CMD_LEN];    // 2d array for command line input
int i, j;   // variable for iteration

// return 1 if input is invalid (skip to next input)
int parse(char* buf) {
    argc=0; // initialization
    if (buf==NULL) {return 1;}
    for (i = 0, j = 0; i < strlen(buf); ++i) {
        if (buf[i] == EOF) {return 1;}
        else if (buf[i] != ' ') {argArray[argc][j++] = buf[i];} 
        else {
            if (j != 0) {
                argArray[argc][j] = '\0';
                ++argc;
                j = 0;
            }
        }
    }
    if (j != 0) {argArray[argc][j] = '\0';}
    return 0;
}


int main() {
    char* buffer;
    while (1) {
        char* buffer;
        buffer=readline(">> ");
        if (parse(buffer)==1) {printf("\n"); continue;}    // handle ctrl-d and parse input
        // printf("%s\n", buffer);
        // for (j=0;j<argc+1;j++) {printf("%s\n", argArray[j]);}
        free(buffer);
    }

    return 0;
}