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
	int id;
};


struct LinkedList{
	struct Node* head;
	struct Node* tail;
	int i ;
};


//methods 
struct LinkedList* init_list();
void free_nodes(struct Node* item);
void free_list(struct LinkedList *aaa);
void insertAtHead(struct LinkedList* jobList, struct Node* node);
struct Node* createNewNode(struct job* jobtoadd);
void insertAtTail(struct LinkedList* jobList, struct Node* node);
struct Node* findJobByJobId(struct LinkedList* jobList, int id);
struct Node* findJobByPID(struct LinkedList* jobList, int id);
void printForward(struct LinkedList* jobList);
void printBackward(struct LinkedList* jobList);
void printJobs();


#endif
