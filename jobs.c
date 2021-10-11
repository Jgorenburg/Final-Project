#include "jobs.h"

struct job* initJob(pid_t ID,  char* input, struct termios *ioSettings) {
	struct job* newJob = (struct job *)malloc(sizeof(struct job));

	newJob->pid = ID;
	newJob->grp_id = ID;
	newJob->input = (char *)malloc((1 + strlen(input)) * sizeof(char));
	strcpy(newJob->input, input);
	newJob->state = running;
	newJob->status = fg;
	newJob->ioSettings = ioSettings;

	return newJob;
}

// Destructor for jobs
void free_job(struct job* item) {
    free(item->input);
	free(item);
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
struct termios *getTermios(const struct job j) {
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
void setTermios(struct job *j, struct termios *t){
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









