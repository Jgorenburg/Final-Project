#include "executor.h"

#ifndef NULL
#define NULL (void *) 0
#endif

// returns -1 if c is not a special char
// and the index of c in specialChar if it is
int isSpecChar(char c) {
	//char *specialChars = "&%;|><";
	for (int i = 0; i < strlen(specialChars); i++) {
		if (c == specialChars[i]) {
			return i;
		}
	}
	return -1;
}

void specChar(char c) {
	switch(c) {
		case '&' :
			commandLoc = BG;
		default :
			printf("special char %c not currently handled", c);
	}	
}


void runProg(char* args[]) {
	pid_t pid;

	printf("before fork\n");

	// child's code

	if ((pid = fork()) == 0) {

		char * command = args[0];
		//command = strtok(args[0], "\0");

		int err = execvp(command, args);		
		if (err == -1) {
			printf("error: did not recognize the command");
		}
		exit(0);
	}

	// parents code
	else if (pid > 0) {

		// TODO: job stuff

		// the shell does not stop if the process is running in the bg
		if (commandLoc == FG) {
			pause();
		}
		else {
			printf("error: fork did not run properly");
		}
	}
}

void execute() {

	commandLoc = FG;

	int i = 0;
	int startPos = i;
	//	if (builtIn(argArray[startPos])) {

	// implement built in functions
	if (0){
	}
	else {
		while (i < argc) {
			int pos;
			if ((pos = isSpecChar(argArray[i][0])) >= 0) {
				// implement special chars
				specChar(argArray[i][0]);
				int numArgs = i - startPos;
				if (numArgs > 0) {
					char args[numArgs + 1][MAX_CHARS_PER_LINE];
					for (int j = 0; j < numArgs; j++) {
						strcpy(args[j], argArray[startPos + j]);
					}
					char *empty = "";
					strcpy(args[numArgs], empty);
				//	runProg(args);	
					startPos = i + 1;
				}
				startPos = i + 1;
			}
			else {
				i++;
			}			
		}
		int numArgs = i - startPos;
		if (numArgs > 0) {
			//	char args[numArgs + 1][MAX_CHARS_PER_LINE];
			char *args[numArgs + 1];
			for (int j = 0; j <= numArgs; j++) {
				args[j] = (char*)malloc(MAX_CHARS_PER_LINE * sizeof(char));
			}
			
			for (int j = 0; j < numArgs; j++) {
				strcpy(args[j], argArray[startPos + j]);
			}
			//char *empty = "";
			//strcpy(args[numArgs], empty);
			args[numArgs] = NULL;
			runProg(args);

			/*for (int j = 0; j <= numArgs; j++) {
				free(args[j]);
			}*/
			free(args);
		}
	}	
}

