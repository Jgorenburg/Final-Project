#include "executor.h"

#ifndef NULL
#define NULL (void *) 0
#endif

// returns -1 if c is not a special char
// and the index of c in specialChar if it is
int isSpecChar(char c) {
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
			break;
		case ';' :
			// do nothing
			break;
		default :
			printf("special char %c not currently handled", c);
	}	
}


void runProg(char* args[]) {
	pid_t pid;
	int status;

	// child's code

	if ((pid = fork()) == 0) {
		//default signal handeling for child

		// setpgid(0, 0);
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

		// TODO: figure out giving input to job
		struct termios job_termios;

		if(tcgetattr(STDIN_FILENO, &job_termios) < 0) {
			printf("error: assigning termios failed");
			return;
		}

		struct job* newJob = initJob(pid, "placeholder", &job_termios);
		setState(newJob, commandLoc);
		struct Node* jobNode = createNewNode(newJob);
		insertAtHead(joblist, jobNode);


		// the shell does not stop if the process is running in the bg
		if (commandLoc == FG) {
			waitpid(pid, &status, 0); 
		}
	}
}


bool builtIn(char* input, int argc, char*argv[]){
	printf("%s\n", input);
	if(strcmp(input, "kill") == 0){
		if(argc == 1){
			printf("can't kill as no job pid given");
		}
		else if(1 == 0 ){

		}
		return true;
	} else if(strcmp(input, "bg") == 0){
		if(joblist->i == 0){
			printf("no jobs in the background");
		}
		else if (argc > 0) {
			// TODO: bg specific job
			int id;
			if (sscanf(argv[1], "%d", &id) == 0) {
				printf("error: %s not an int\n", argv[1]);
			}
			else {
				struct Node* newBG = findJobByJobId(joblist, id);
				if (newBG == NULL) {
					printf("error: %d not a valid job\n", id);
				}
				else if (newBG->data->status != suspended) {
					printf("job %d is already running in the background\n", id);
				}
				else {
					newBG->data->state = bg;
					newBG->data->status = running;
					kill(newBG->data->pid, SIGCONT);	
				}
			} 
		}
		else{
			struct Node* temp = joblist->head;
			while(temp->data->status != suspended && temp != NULL){
				printf("%d", temp->data->status );
				temp = temp->next;
			}
			if(temp->data->status == suspended){
				kill(temp->data->pid, SIGCONT);
			}
		}
		return true;
	} else if(strcmp(input, "fg") == 0){
		return true;
	} else if(strcmp(input, "exit") == 0){
		exit(0);
	}
	else{
		return false;
	}	

}
void execute() {

	commandLoc = FG;

	int i = 0;
	int startPos = i;
	while (i < argc) {
		int pos;
		if ((pos = isSpecChar(argArray[i][0])) >= 0) {
			// implement special chars
			specChar(argArray[i][0]);
			int numArgs = i - startPos;
			if (numArgs > 0) {

				char *args[numArgs + 1];
				for (int j = 0; j <= numArgs; j++) {
					args[j] = (char*)malloc(MAX_CHARS_PER_LINE * sizeof(char));
				}
				for (int j = 0; j < numArgs; j++) {
					strcpy(args[j], argArray[startPos + j]);
				}
				args[numArgs] = NULL;

				if (!builtIn(argArray[startPos], numArgs, args)){
					runProg(args);	
				}

				for (int j = 0; j <= numArgs; j++) {
					free(args[j]);
				}

			}
			startPos = i + 1;
		}
		i++;	
	}
	int numArgs = i - startPos;
	if (numArgs > 0) {
		char *args[numArgs + 1];
		for (int j = 0; j <= numArgs; j++) {
			args[j] = (char*)malloc(MAX_CHARS_PER_LINE * sizeof(char));
		}

		for (int j = 0; j < numArgs; j++) {
			strcpy(args[j], argArray[startPos + j]);
		}
		args[numArgs] = NULL;

		if (!builtIn(argArray[startPos], numArgs, args)) {
			runProg(args);
		}
		for (int j = 0; j <= numArgs; j++) {
			free(args[j]);
		}	
	}	

}

