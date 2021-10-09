#ifndef __JOB_H
#define __JOB_H
#include <sys/types.h>

struct job{
    pid_t pid;
    gid_t gid; //group id
    char ** t_arr;
    int background;
    int running;
    int jobid;
};

typedef struct job jobInfo;

jobInfo* initJob();

#endif