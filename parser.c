// parser program in C 
// that process command line input for the shell

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#ifndef NULL
#define NULL (void *) 0
#endif
enum{DOWN,UP};  // flag values for parsing input
#define MAX_CMD 15
#define BUFFSIZE 256
#define MAX_CMD_LEN 100

// extern variables
int argc;    // number of valid arguments
char* argv[MAX_CMD]; // a char array to store tokenized command argument
char command[MAX_CMD][MAX_CMD_LEN];  // 2d array to check for special symbols
char buf[BUFFSIZE];  // buffer to read from stdin
char buf_copy[BUFFSIZE];    // copy of buffer, need it because we rewrite buffer when checking for special symbols
int i, j;    // variables for iterating through arguments
// extern pid_t pid;

// still working on it
// signal handler so that if child gets signal, it sends signal to parent
// void sig_handler(int signum){
//     kill(pid,SIGINT);
// }

// function to read from stdin and return length of user input (to skip empty input)
int read_input(char buf[]) {
    // initialization
    memset(buf, 0x00, BUFFSIZE);
    memset(buf_copy, 0x00, BUFFSIZE);  
    // read from stdin      
    fgets(buf, BUFFSIZE, stdin);
    if (buf==NULL) {return 0;}
    // changing ending character from input "\n" to '\0' in order to mark termination
    buf[strlen(buf)-1] = '\0';
    return strlen(buf);
}

// function to parse through input and identify valid arguments
void parse_info(char* buf) {
    // initialization - default tombstoning symbol is '\0'
    for (i=0; i<MAX_CMD; i++) {
        argv[i] = NULL;
        for (j=0; j<MAX_CMD_LEN; j++) {command[i][j] = '\0';}
    }
    argc = 0;
    strcpy(buf_copy, buf);  // make a copy of original buffer for future reference
    // to build an array of command arguments:
    // command[i] is a char array of one command argument, such as 'cd', 'ls', '-n'
    int len = strlen(buf);
    for (i=0, j=0; i<len; ++i) {
        if (buf[i]!=' ') {command[argc][j++] = buf[i];}
        else {
            // every whitespace in buffer indicates the end of the previous command argument
            if (j!=0) {
                command[argc][j] = '\0';
                ++argc;
                j = 0;  // reset iter var for the next command argument
            }
        }
    }
    if (j!=0) {command[argc][j] = '\0';}  // tombstone the end of last argument
    // iterate through the buffer and 2d array command[][] to tokenize arguments into 1d array
    // package the command line arguments according to execcvp
    // i.e. argv[i] is a string of one command argument, such as 'cd', 'ls', '-n'
    argc = 0;
    int flag = DOWN;   // to keep track of valid and invalid info (delimitors)
    for (i=DOWN; buf[i]!='\0'; i++) {
        if (flag==DOWN && !isspace(buf[i])) {
            flag = UP;
            argv[argc++] = buf+i;
        } 
        else if (flag==UP && isspace(buf[i])) {
            flag = DOWN;
            buf[i] = '\0';  // replace whitespace with tombstone in buffer
        }
    }
    argv[argc] = NULL;  // tombstone termination
}

// to check for special symbols DO:
// still working on ; (need help)
//     for (i = 0; i < MAX_CMD; i++) {
//         if (strcmp(command[j], "&") == 0) {
//             strcpy(buf, backupBuf);
//             // use some function to execute
//             return;
//         }
//     }


int main() {
    while (1) {
        printf(">> "); // prompt
        if (read_input(buf)==0) {continue;} // read from stdin and skip null(whitespace) input
        strcpy(buf_copy, buf);  // make a copy of input because we change buffer content when dealing with pipe and redirections
        parse_info(buf);    // tokenize and extract valid information
//         exec_cmd(argc, argv);   // execute command arguments
        argc = 0;
    }

    return 0;
}
