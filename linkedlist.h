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
    struct Node* head;
    struct Node* tail;
};


//methods 
void insertAtHead(struct Node* head, struct Node* tail, struct Node* node);
struct Node* createNewNode(struct LinkedList* jobList, struct job* jobtoadd);
void insertAtTail(struct Node* head, struct Node* tail, struct Node* node);
struct Node* findJobByJobId(struct LinkedList* jobList, int id);
void printForward(struct LinkedList* jobList);
void printBackward(struct LinkedList* jobList);


#endif
