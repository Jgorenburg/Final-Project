#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "job.h"

jobInfo* initJob(){
    jobInfo *job = (jobInfo *) malloc(sizeof(jobInfo));
    if(job == NULL){
        printf("could not create/malloc the job\n");
    }

}