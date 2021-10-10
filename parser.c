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
#define SYMBOLS "&%;|><"

// global variables
int argc;   // number of arguments
char argArray[MAX_CMD][MAX_CMD_LEN];    // 2d array for command line input
int i, j;   // variable for iteration

// initialize argArray
void init_arg() {
    for (i=0;i<MAX_CMD;i++) {
        for (j=0;j<MAX_CMD_LEN;j++) {
            argArray[i][j]='\0';
        }
    }
}

// checks if character is a specials symbol
// return 0 if it is, else return 1
int special_symbol(char item) {
    for (int i=0;i<strlen(SYMBOLS)-1;i++) {
        if (item==SYMBOLS[i]) {return 0;}
    }
    return 1;
}

// return 1 if input is invalid (skip to next input)
// else return 0
int parse(char* buf) {
    argc=0; // initialization
    if (buf==NULL || buf[0] == EOF) {return 1;}
    for (i = 0, j = 0; i < strlen(buf); i++) {
        if (special_symbol(buf[i])==0) {
            if (j != 0) {
                argArray[argc++][j] = '\0';
                j = 0;
            }
            argArray[argc++][j] = buf[i];
        }
        else if (buf[i] != ' ') {argArray[argc][j++] = buf[i];} 
        else {
            if (j != 0) {
                argArray[argc++][j] = '\0';
                j = 0;
            }
        }
    }
    if (j != 0) {argArray[argc++][j] = '\0';}
    return 0;
}

int main() {
    char* buffer;
    while (1) {
        char* buffer;
        buffer=readline(">> ");
        init_arg();
        if (parse(buffer)==1) {printf("\n"); continue;}    // handle ctrl-d and parse input
        printf("%d\n", argc); 
        for (j=0;j<argc+1;j++) {printf("%s\n", argArray[j]);}
        free(buffer);
    }

    return 0;
}
