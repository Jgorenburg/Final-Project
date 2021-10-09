#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

enum procLoc{BG, FG};

procLoc commandLoc = FG;


int runProg(char *args[]);
int main();
