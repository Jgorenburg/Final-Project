#include "jobs.h"

struct job initJob(pid_t ID,  char* input, struct Termios *ioSettings) {
	struct job newJob;

	newJob.pid = ID;
	newJob.input = input;
	newJob.state = running;
	newJob.status = fg;
	newJob.ioSettings = ioSettings;

	return newJob;
}


int getID(const struct job j) {
	return j.pid;
}
int getGrpID(const struct job j) {
	return j.grp_id;
}
enum State getState(const struct job j) {
	return j.state;
}
char* getInput(const struct job j) {
	return j.input;
}
enum Status getStatus(const struct job j) {
	return j.status;
}
struct Termios *getTermios(const struct job j) {
	return j.ioSettings;
}



void setID(struct job *j, pid_t id){
	j->pid = id;
}
void setGrpID(struct job *j, int ID){
	j->grp_id = ID;
}
void setState(struct job *j, enum State s){
	j->state = s;
}
void setStatus(struct job *j, enum Status s){
	j->status = s;
}
void setTermios(struct job *j, struct Termios *t){
	j->ioSettings = t;
}



void setBackground(struct job *j) {
	j->state = bg;
}
void setForeground(struct job *j) {
	j->state = fg;
}
void pauseJob(struct job *j) {
	j->status = suspended;
}
// void resumeJob(struct job *j) {
// 	j->status = running;
// }









