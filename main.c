#include <termios.h>
#include <signal.h>
#include "main.h"
#include "linkedlist.h"

// #include <signal.h>
// #include <termios.h>
// #ifndef NULL
// #define NULL (void *) 0
// #endif

//global variuable 
int shell_id;
struct termios* shell_terminal_settings;


// // signal handler for SIGCHLD
// static void signal_handler(int sig, siginfo_t *si, void *unused){
//     // check siginfo_t (si->si_code) then do cases
//     printf("child id is: %d; status is: %d\n", si->si_pid, si->si_code);
//     int child_code = si->si_code;
//     if (child_code==CLD_EXITED) {
//         // update(delete) the job from job list
//     }
//     if (child_code==CLD_KILLED || child_code==CLD_DUMPED) {
//         // child has terminated abnormally
//         // update(delete) the job from job list
//     }
//     if (child_code==CLD_TRAPPED) {
//         // child has trapped (do not know what this means)
//         // do we do anything?
//     }
//     if (child_code==CLD_STOPPED) {
//         // check if it is a foreground job
//         // if it is, put it in bg and update(add it to) job list 
//     }
//     if (child_code==CLD_CONTINUED) {
//         // Stopped child has continued.
//         // change status to one
//         // put it in fg and delete it from job list
//     }
//     // kill(si->si_pid, SIGCONT);
//     // sleep(10);
// }


int main() {
/*
=======
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
		switch(status){
			case CLD_EXITED:
			case CLD_DUMPED:
			case CLD_KILLED: 
				printf("process %d killed successfully.", temp.data->pid);
				sigset_t set;
				sigemptyset(&set);
				sigaddset(&set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &set, SIGCHLD);
				destructoreJob(&temp);
				sigprocmask(SIG_UNBLOCK, &set, SIGCHLD);
				break;
			case CLD_STOPPED:
				sigset_t set;
				sigemptyset(&set);
				sigaddset(&set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &set, SIGCHLD);
				temp.data->status = suspended;
				printf("process %d suspended successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &set, SIGCHLD);
				break;
			case CLD_CONTINUED:
				sigset_t set;
				sigemptyset(&set);
				sigaddset(&set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &set, SIGCHLD);
				temp.data->status = running;
				printf("process %d resumed successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &set, SIGCHLD);
				break; 
			default:
				break;
		}
	}
}


int main() {

>>>>>>> e3589cb20b03c771e419369414f9ba70f7f4fcd9
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

*/


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
