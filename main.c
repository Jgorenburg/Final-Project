#include "main.h"

// #include <signal.h>
// #include <termios.h>
// #ifndef NULL
// #define NULL (void *) 0
// #endif

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


void main() {
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
}
