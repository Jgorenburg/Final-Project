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

	// child's code
	if ((pid = fork()) == 0) {	
		// char * command = strtok(args[0], "\0");
		// printf("command char is: ");
		// for (int k=0;k<strlen(command);k++) {
		// 	printf("%c, ", command[k]);
		// }
		// printf("argument list is: ");
		// for (int j = 0; j < strlen(*args); j++) {
		// 	printf("%c; ",*args[j]);
		// }
		// int err = execvp(command, args);
		int err = execvp(args[0], args);		
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
					char *args[numArgs];
					runProg(args);	
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
			char *args[100];
			for (int j = 0; j < numArgs+1; j++) {
				if (j==numArgs) {
					args[j]='\0';
				}
				else {
					for (int k=0;k<strlen(argArray[startPos + j]);k++) {
						args[j][k]=argArray[startPos + j][k];
					}
				}
			}

			printf("length of argument list is: %d\n",numArgs);
			printf("argument list is: \n");
			for (int j = 0; j < numArgs; j++) {
				printf("%s,\n",args[j]);
			}
			runProg(args);
			// int err = execvp(args[0], args);	
			// if (err == -1) {
			// 	printf("error: did not recognize the command");
			// }
		}	
	}
}
