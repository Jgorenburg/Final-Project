#include "main.h"

// #include <signal.h>
// #include <termios.h>
// #ifndef NULL
// #define NULL (void *) 0
// #endif


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
