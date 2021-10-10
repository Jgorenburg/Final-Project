#ifndef LINKEDLIST_H
#define LINKEDLIST_H

//doubly linked list
#include<stdio.h>
#include<stdlib.h>

#include "jobs.h"
#include "executor.h"

struct Node {
        struct job* data; //the job struct storage
        struct Node* next;
        struct Node* prev;
};


struct LinkedList{
<<<<<<< HEAD
    struct Node* head;
    struct Node* tail;
=======
	struct Node* head;
	struct Node* tail;
	int i;
>>>>>>> b18c53ca69dc339be42905d316e62cf3a765bef5
};


//methods 
void insertAtHead(struct LinkedList* jobList, struct Node* node);
struct Node* createNewNode(struct LinkedList* jobList, struct job* jobtoadd);
void insertAtTail(struct LinkedList* jobList, struct Node* node);
struct Node* findJobByJobId(struct LinkedList* jobList, int id);
void printForward(struct LinkedList* jobList);
void printBackward(struct LinkedList* jobList);


#endif
