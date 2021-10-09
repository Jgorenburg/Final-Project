#include "commands.h"




int runProg(char *args[]) {
	pid_t pid;

	// TODO: job stuff

	// child's code
	if ((pid = fork()) == 0) {	
		int err = execvp(args[0], args);		
		if (err == -1) {
			printf("error: did not recognize the command");
		}
		exit(status);
	}

	// parents code
	else if (pid > 0) {
		// the shell does not stop if the process is running in the bg
		if (commandLoc == FG) {
			pause();
		}
		else {
			printf("error: fork did not run properly");
		}
	}
}

void main() {

	pid_t pid;

	// get parser output
	// place in argv 

	int i = 0;
	int startPos = i;
	if (builtIn(argv[startPos])) {
		// implement built in functions
	}
	else {
		while (i < argc) {
			if (specialChar(argv[i]) {
				// implement special chars
			}
			++i;			
		}
	}
}






