#include "executor.h"
#include <string.h>
#include <fcntl.h>

#ifndef NULL
#define NULL (void *) 0
#endif

// returns -1 if c is not a special char
// and the index of c in specialChar if it is
int isSpecChar(char c) {
	for (int i = 0; i < strlen(specialChars); i++) {
		if (c != '%' && c == specialChars[i]) {
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
		// reset default signal handeling for child
		for (int signum=0;signum<=64;signum++) {
			signal(signum, SIG_DFL);
		}
		// setpgid(0, 0);
		char * command = args[0];
		//command = strtok(args[0], "\0");
		int err = execvp(command, args);		
		if (err == -1) {
			printf("error: did not recognize the command");
		}
		quitting = true;
		//exit(0);
	}

	// parents code
	else if (pid > 0) {

		// TODO: figure out giving input to job
		struct termios job_termios;

		if(tcgetattr(STDIN_FILENO, &job_termios) < 0) {
			printf("error: assigning termios failed");
			return;
		}

		int size = 0;
		while (args[size] != NULL) {
			size++;
		}
		char * input = malloc(argc * MAX_CHARS_PER_LINE * sizeof(char));

		strcpy(input, args[0]);
		for (int i = 1; i < size; i++) {
			strcat(input, " ");
			strcat(input, args[i]);
		}

		struct job* newJob = initJob(pid, input, &job_termios);
		setState(newJob, commandLoc);
		struct Node* jobNode = createNewNode(newJob);
		insertAtHead(joblist, jobNode);
		free(input);

		// the shell does not stop if the process is running in the bg
		if (commandLoc == FG) {
			waitpid(pid, &status, 0); 
		}
	}
}


bool builtIn(char* input, int argc, char*argv[]){
	if(strcmp(input, "kill") == 0){
		if(argc < 2){
			printf("can't kill as no job pid given");
		}
		else {
			int id;
			if (argc > 2 && strcmp(argv[1], "-9") == 0) {
				if (strcmp(argv[2], "%") != 0) {
					printf("error: need a %% \n");
				}	
				else if (argc < 4) {
					printf("error: not enough args");
				}	
				else if (sscanf(argv[3], "%d", &id) == 0) {
					printf("error: %s is not an int\n", argv[3]);
				}
				else {
					struct Node* job = findJobByJobId(joblist, id);
					if (job == NULL) {
						printf("error: %d not a valid job\n", id);
					}
					else {
						kill(job->data->pid, SIGKILL);
					}
				}
			}
			else {
				if (strcmp(argv[1], "%") != 0) {
					printf("error: need a %% \n");
				}
				else if (argc < 3) {
					printf("error: not enough args");
				}
				else if (sscanf(argv[2], "%d", &id) == 0) {
					printf("error: %s is not an int\n", argv[2]);
				}
				else {
					struct Node* job = findJobByJobId(joblist, id);
					if (job == NULL) {
						printf("error: %d not a valid job\n", id);
					}
					else {
						kill(job->data->pid, SIGTERM);
					}
				}
			}
		}
		return true;
	} else if(strcmp(input, "jobs") == 0) {
		printJobs();
		return true;
	} else if(strcmp(input, "bg") == 0){
		if(joblist->i == 0){
			printf("no jobs to background");
		}
		int lookPos = 1;
		if (argc > 2 && strcmp(argv[1], "%") == 0) {
			lookPos++;
		}
		if (argc > lookPos) {
			int id;
			if (sscanf(argv[lookPos], "%d", &id) == 0) {
				printf("error: %s not an int\n", argv[lookPos]);
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
			while(temp != NULL && temp->data->status != suspended){
				temp = temp->next;
			}
			if(temp != NULL && temp->data->status == suspended){
				temp->data->state = bg;
				temp->data->status = running;
				kill(temp->data->pid, SIGCONT);
			}
		}
		return true;
	} else if(strcmp(input, "fg") == 0){
		int status;
		if(joblist->i == 0){
			printf("no jobs to foreground");
		}
		int lookPos = 1;
		if (argc > 2 && strcmp(argv[1], "%") == 0) {
			lookPos++;
		}
		if (argc > lookPos) {
			int id;
			if (sscanf(argv[lookPos], "%d", &id) == 0) {
				printf("error: %s not an int\n", argv[lookPos]);
			}
			else {
				struct Node* newFG = findJobByJobId(joblist, id);
				if (newFG == NULL) {
					printf("error: %d not a valid job\n", id);
				}
				else {
					if (newFG->data->status == suspended){
						kill(newFG->data->pid, SIGCONT);
					}
					newFG->data->state = fg;
					newFG->data->status = running;
					int fg_pid = newFG->data->pid;
					waitpid(fg_pid, &status, 0);
					waitpid(fg_pid, &status, 0);

				}
			} 
		}
		else{
			struct Node* temp = joblist->head;
			if(temp != NULL){
				if (temp->data->status == suspended){
					kill(temp->data->pid, SIGCONT);
				}
				temp->data->state = fg;
				temp->data->status = running;
				int fg_pid = temp->data->pid;
				waitpid(fg_pid, &status, 0);
				waitpid(fg_pid, &status, 0);
			}	
		}
		return true;
	} else if(strcmp(input, "exit") == 0){
		// can't exit here cause we need to free args in execute()
		quitting = true;
		return true;
	}

	// new commands for hw 7
	else if (strcmp(input, "ls") == 0) {
		bool suffix = false;
		bool detail = false;
		if (argc > 1) {
			for (int i = 1; i < argc; i++) {
				suffix |= ((strcmp(argv[i], "-F") == 0) || (strcmp(argv[i], "-lF") == 0)
						|| (strcmp(argv[i], "-Fl") == 0));
				detail |= ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "-lF") == 0)
						|| (strcmp(argv[i], "-Fl") == 0));
			}
		}
	} else if (strcmp(input, "chmod") == 0) {
		// if (argc < 3) {
		// 	printf("usage: chmod <file path> <permission>\n");
		// } else {
		// 	char *filename = malloc(strlen(argv[1])+1);
		// 	filename = argv[1];
		// 	char *permission = malloc(strlen(argv[2])+1);
		// 	permission = argv[2];
		// }
	} else if (strcmp(input, "mkdir") == 0) {
		if (argc < 2) {
			printf("error: no directory provided to be built\n");
		}	
		else {
			char *dir_name = argv[1];		
			if (f_mkdir(dir_name, DEFAULT_DIR_PERMISSION) < 0) {
				free(dir_name);
			}
		}
		return true;
	} else if (strcmp(input, "rmdir") == 0) {
	} else if (strcmp(input, "cd") == 0) {
		if (argc < 2) {
			curDir = 0;
		}	
		else {
			char *dir_name = argv[1];		
			curDir = f_moveDir(dir_name);
		}
		return true;
	} else if (strcmp(input, "pwd") == 0) {		
		struct inode pwd_inode = dimage->inodes[curDir];

		find_pwd_dir();
		return true;
	} else if (strcmp(input, "cat") == 0) {
	} else if (strcmp(input, "rm") == 0) {
	} else if (strcmp(input, "mount") == 0) {
	} else if (strcmp(input, "unmount") == 0) {
	}
	else{
		return false;
	}	
	return false;
}
void execute() {

	commandLoc = FG;
	quitting = false;	

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
				if (quitting) {	
					free_diskimage(dimage);
					free_list(joblist);
					fclose(disk);
					exit(0);
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
		if (quitting) {	
			free_list(joblist);
			exit(0);
		}
	}	

}

