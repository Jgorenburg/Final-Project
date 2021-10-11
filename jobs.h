#ifndef JOBS_H
#define JOBS_H

#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum State {bg, fg};
enum Status {suspended, running};

struct job {
	pid_t pid;
	int grp_id;
	enum State state;
	char* input;
	enum Status status;
	struct termios *ioSettings;
};

// makes a new job for the current process
// and adds it to the job list
struct job* initJob(pid_t ID,  char* input, struct termios *ioSettings);

// returns element for the given job
int getID(const struct job j);
void free_job(struct job* item);
int getGrpID(const struct job j);
enum State getState(const struct job j);
char* getInput(const struct job j);
enum Status getStatus(const struct job j);
struct termios* getTermios(const struct job j);

void setID(struct job *j, pid_t id);
void setGrpID(struct job *j, int ID);
void setState(struct job *j, enum State s);
void setStatus(struct job *j, enum Status s);
void setTermios(struct job *j, struct termios *t);

void setBackground(struct job *j);
void setForeground(struct job *j);
void pauseJob(struct job *j);
void resumeJob(struct job *j);


#endif 
