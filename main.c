#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


#include "main.h"
#include "linkedlist.h"


//global variuable 
int shell_id;
struct termios* shell_terminal_settings;


//destructor jobs
void destructoreJob(struct Node* temp){
	struct Node localNode = *temp;
	localNode.prev = localNode.next;
	struct job* givenJob = localNode.data;
	free(givenJob->input);
	free(givenJob->ioSettings);
	free(temp);
}

// signal handler for SIGCHLD
static void signal_action_handler(int sig, siginfo_t *si, void *unused){
	struct Node temp = *findJobByJobId(joblist, (int) si->si_pid);
	//now cases
	if(sig == SIGCHLD){
		int status = si->si_code;
		sigset_t new_set, old_set;
		switch(status){
			case CLD_EXITED:
			case CLD_DUMPED:
			case CLD_KILLED: 
				printf("process %d killed successfully.", temp.data->pid);
				// sigset_t new_set, old_set;
				sigemptyset(&new_set);
				sigaddset(&new_set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &new_set, &old_set);
				destructoreJob(&temp);
				sigprocmask(SIG_UNBLOCK, &old_set, NULL);
				break;
			case CLD_STOPPED:
				// sigset_t new_set, old_set;
				sigemptyset(&new_set);
				sigaddset(&new_set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &new_set, &old_set);
				temp.data->status = suspended;
				// kill(temp.data->pid, SIGCONT);
				insertAtHead(joblist, &temp);
				printf("process %d suspended successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &old_set, NULL);
				break;
			case CLD_CONTINUED:
				// sigset_t new_set, old_set;
				sigemptyset(&new_set);
				sigaddset(&new_set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &new_set, &old_set);
				temp.data->status = running;
				printf("process %d resumed successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &old_set, NULL);
				break; 
			default:
				break;
		}
	}
}


int main() {
	joblist->head = NULL;
	joblist->tail = NULL;
	joblist->i = 0;

	//mallocing space for shell settings termios
	shell_terminal_settings = (struct termios *) malloc(sizeof(struct termios));
	shell_id = getpid(); //stoding shellpid

	//shell signals
	signal(SIGINT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	//putiing shell in its out group 
	if(setpgid(shell_id, shell_id) < 0){
		perror("can't give shell a unique group id");
		exit(1);
	}

	//shell gets the control 
	tcsetpgrp(STDIN_FILENO, shell_id);
	tcgetattr(STDIN_FILENO, shell_terminal_settings); //saving shell settings

	struct sigaction sigact;
	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = signal_action_handler;
	sigaction(SIGCHLD, &sigact, NULL);

	joblist = (struct LinkedList*)malloc(sizeof(*joblist));
	
// 	// store settings for shell (outside the while loop) in a global termios (pid and termios)
// 	// in a global termios
// 	struct termios shell_attr;
// 	tcgetattr(0, &shell_attr);
// 	// ignore SIGQUIT and SIGTERM, SIGINT, SIGTSTP so the shell cannot be interrupted
// 	signal(SIGQUIT,SIG_IGN);
// 	signal(SIGTERM,SIG_IGN);
// 	signal(SIGTSTP,SIG_IGN);
// 	// then the sigaction for child processes
// 	struct sigaction sa;
// 	sa.sa_flags = SA_SIGINFO; // | SA_NODEFER ;                                                                                                                                                         
// 	sigemptyset(&sa.sa_mask);
// 	sa.sa_sigaction = sigactionhandler;
// 	if (sigaction(SIGCHLD, &sa, NULL) == -1){
// 	printf("failed to register sigaction handler");
// 	exit(EXIT_FAILURE);
// 	}


	while(1) {
		parserMain();	
		execute();	
	}
	return 1;
}
