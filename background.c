#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

enum procLoc{BG, FG};

procLoc commandLoc = FG;


void main(int argc, char *argv[]) {

	else if ((pid = fork()) == 0) {
		if (!commandLoc) {
			//do job stuff
			int err = execvp(command, args);
					
			if (err == -1) {
				printf("error: did not recognize the command");
			}
			exit(status);
		

	}

	// parent waits for child's command to execute
	else if (pid > 0) {
		waitpid(-1, &status, 0);
	}
	else {
		printf("error: fork did not run properly");
	}


