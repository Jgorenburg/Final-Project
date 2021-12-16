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
	if (localNode.next == NULL){
		if (localNode.prev == NULL) {
			joblist->head = NULL;
			joblist->tail = NULL;
		}
		else {
			localNode.prev->next = NULL;
			joblist->tail = localNode.prev;
		}
	}
	else if (localNode.prev == NULL) {
		localNode.next->prev = NULL;
		joblist->head = localNode.next;
	}
	else {
		localNode.prev->next = localNode.next;
		localNode.next->prev = localNode.prev;
	}
	struct job* givenJob = localNode.data;
	free(givenJob->input);
	free(givenJob);
//	free(temp);
}

// signal handler for SIGCHLD
static void signal_action_handler(int sig, siginfo_t *si, void *unused){
	struct Node temp = *findJobByPID(joblist, (int) si->si_pid);
	//now cases
	if(sig == SIGCHLD){
		int status = si->si_code;
		sigset_t new_set, old_set;
		switch(status){
			case CLD_EXITED:
			case CLD_DUMPED:
			case CLD_KILLED: 
				//printf("process %d killed successfully.", temp.data->pid);
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
			//	printf("process %d suspended successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &old_set, NULL);
				break;
			case CLD_CONTINUED:
				// sigset_t new_set, old_set;
				sigemptyset(&new_set);
				sigaddset(&new_set, SIGCHLD);
				sigprocmask(SIG_BLOCK, &new_set, &old_set);
				temp.data->status = running;
			//	printf("process %d resumed successfully.", temp.data->pid);
				sigprocmask(SIG_UNBLOCK, &old_set, NULL);
				break; 
			default:
				break;
		}
	}
}

// singal hanlder for SIGTSTP (ctrl-z)
void sig_handler_ctrlz(int signum){
	// ignore SIGQUIT, SIGTERM and SIGTSTP(process suspension with Control-Z) 
	// so the shell cannot be interrupted
	signal(signum,SIG_IGN);
	// send a SIGSTOP signal to the process currently executing in the 
	// foreground (depending on implementation-->we don't need to). 
	struct Node* iter=joblist->head;
	while (iter!=NULL) {
		if (iter->data->state==fg) {
			kill(iter->data->pid, SIGSTOP);
			iter->data->status=suspended;
			iter->data->state=bg;
			// return with a new prompt
			parserMain();	
			execute();
			iter = NULL;	
		}
		else {
			iter=iter->next;	// move onto the next on the job list
		}
	}
}

void set_dir_img(struct diskimage *di, FILE *dsk) {
	di->id = (int)dsk;
	struct superblock *tempSB = malloc(sizeof(struct superblock));
	fseek(dsk, 512, SEEK_SET);
	fread(tempSB, sizeof(struct superblock), 1, dsk); 
	di->sb = *tempSB;
	
	int numInodes = ((tempSB->data_offset - tempSB->inode_offset) * tempSB->size) / sizeof(struct inode) + 1;
	di->inodes = malloc(numInodes * sizeof(struct inode));
	int loc = INODE_OFFSET; 
	for (int i = 0; i < numInodes; i++) {			
		struct inode *new_inode = malloc(sizeof(struct inode));
		fseek(dsk, loc, SEEK_SET);
		fread(new_inode, sizeof(struct inode), 1, dsk);
		di->inodes[i] = *new_inode;
		loc += sizeof(struct inode);
		free(new_inode);
	}
	
	di->root = malloc(tempSB->size);
	fseek(dsk, SUPER_OFFSET * 2, SEEK_SET);
	fread(di->root, tempSB->size, 1, dsk);

	int numBlocks = tempSB->swap_offset - tempSB->data_offset;
	di->blocks = malloc(numBlocks * sizeof(char *));
	loc = INODE_OFFSET + (tempSB->data_offset - tempSB->inode_offset) * tempSB->size;
	for (int i = 0; i < numBlocks; i++) {
		char *new_block = malloc(tempSB->size);
		fseek(dsk, loc, SEEK_SET);
		fread(new_block, tempSB->size, 1, dsk);
		di->blocks[i] = new_block;
		loc += tempSB->size;	
	}
	free(tempSB);
}

int main() {
	if (access("DISK", F_OK) != 0) {
		printf("Cannot find the disk file, please run the format command to create one\n");
		return -1;
	}

	disk = fopen("DISK", "r");
	
	char user[20];
	char password[20];
	printf("Please log in\nuser: ");
	fgets(user, 20, stdin);

	if (strcmp(user, "admin") == 0) {
		curDir = 1;
	}
	else {
		curDir = 2;
	}
	
	dimage = malloc(sizeof(struct diskimage));
	set_dir_img(dimage, disk);

	joblist=init_list();

	//mallocing space for shell settings termios
	shell_terminal_settings = (struct termios *) malloc(sizeof(struct termios));
	shell_id = getpid(); //stoding shellpid

	//shell signals
	signal(SIGINT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, sig_handler_ctrlz);	// for ctrl-z

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
	sigfillset(&sigact.sa_mask);
	sigact.sa_sigaction = signal_action_handler;
	sigaction(SIGCHLD, &sigact, NULL);

	while(1) {
		parserMain();	
		execute();	
	}
	return 1;
}
