#include <stdbool.h> 
#include "executor.h"

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


void runProg(char *args[], char *args2[]) {
	pid_t pid;

	// child's code
	if ((pid = fork()) == 0) {
		//setting default signal handling for the child process	
		// signal(SIGINT, SIG_DFL);
        // signal(SIGQUIT, SIG_DFL);
        // signal(SIGTTIN, SIG_DFL);
        // signal(SIGTTOU, SIG_DFL);
        // signal(SIGCHLD, SIG_DFL);

		char * command = strtok(args[0], "\0");
		int err = execvp(command, args2);		
		if (err == -1) {
			printf("error: did not recognize the command");
		}
		exit(0);
	}

	// parents code
	else if (pid > 0) {

		// TODO: figure out giving input to job
		struct termios job_termios;
		
		if(tcgetattr(STDIN_FILENO, &job_termios) < 0) {
        		printf("error: assigning termios failed");
			return;
        	}

		struct job* newJob = initJob(pid, "placeholder", &job_termios);
		joblist->i++;
		setState(newJob, commandLoc);
		struct Node* jobNode = createNewNode(newJob);
		insertAtHead(joblist, jobNode);


		// the shell does not stop if the process is running in the bg
		if (commandLoc == FG) {
			pause();
		}
		// TODO: what is the purpose of this else block?
		else {
			printf("error: fork did not run properly");
		}
	}
}

bool builtIn(char* input){
	printf("inside builtIn");
	if(strcmp(input, "kill")){
		return true;
	} else if(strcmp(input, "fg")){
		return true;
	}
	 else if(strcmp(input, "bg")){
		return true;
	}
	 else if(strcmp(input, "jobs")){
		return true;
	}
	 else if(strcmp(input, "exit")){
		return true;
	}
	else{
		return false;
	}
}
void execute() {
	commandLoc = FG;

	int i = 0;
	int startPos = i;
	if (builtIn(argArray[startPos])) {
		if(strcmp(argArray[startPos], "kill")){
			if(argc == 1){
				printf("can't kill as no job pid given");
			} else if(1 == 0 ){

			}
		} else if(strcmp(argArray[startPos], "bg")){

		}
	
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
					for (int j = 0; j < numArgs; j++) {
						args[j] = argArray[startPos + j];
					}
					runProg(args, args);	
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
			char *args[numArgs];
			for (int j = 0; j < numArgs; j++) {
				args[j] = argArray[startPos + j];
			}
			runProg(args, args);	
		}	
	}
}






