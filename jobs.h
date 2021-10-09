#include <termios.h>

enum State {bg, fg};
enum Status {suspended, running};

struct job {
	pid_t pid;
	int grp_id;
	enum State state;
	char* input;
	enum Status status;
	struct Termios *ioSettings;
};

// makes a new job for the current process
// and adds it to the job list
void initJob(pid_t ID,  char* input, struct Termios *ioSettings);

// returns element for the given job
int getID(const struct job j);
int getGrpID(const struct job j);
enum State getState(const struct job j);
char* getInput(const struct job j);
enum Status getStatus(const struct job j);
struct Termios* getTermios(const struct job j);

void setID(struct job *j, pid_t id);
void setGrpID(struct job *j, int ID);
void setState(struct job *j, enum State s);
void setStatus(struct job *j, enum Status s);
void setTermios(struct job *j, struct Termios *t);

void setBackground(struct job *j);
void setForeground(struct job *j);
void pauseJob(struct job *j);
void resumeJob(struct job *j);


 
