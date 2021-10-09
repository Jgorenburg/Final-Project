#include "jobs.h"

void init_job(int ID, int grpID, char* input, struct Termios *ioSettings) {
	struct job newJob;

	newJob.id = ID;
	newJob.grp_id = grpID;
	newJob.input = input;
	newJob.state = running;
	newJob.status = fg;
	newJob.ioSettings = ioSettings;

	// insert on linked list
	// return way to access job
}


int getID(const struct job j) {
	return j.id;
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



void setID(struct job *j, int id){
	j->id = id;
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
	j->status = stopped;
}
void resumeJob(struct job *j) {
	j->status = running;
}

void main(){
	


}







