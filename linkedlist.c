#include<stdio.h>
#include<stdlib.h>
#include "linkedlist.h"
#include "jobs.h"

struct Node* createNewNode(struct LinkedList* jobList, struct job* jobtoadd){
    struct Node* newJob = (struct Node *) malloc(sizeof(struct Node));
    newJob->data = jobtoadd;
    newJob->next = NULL;
    newJob->prev = NULL;
    return newJob;
}

//Insert at the Head
void insertAtHead(struct Node* head, struct Node* tail, struct Node* node){
    if(head == NULL){
        head = node;
        tail = node;
    }
    else{
        head->prev = node;
        node->next = head;
        head = node;
    }
}

//insert at tail of the doubly linked List
void insertAtTail(struct Node* head, struct Node* tail, struct Node* node){
    if(tail == NULL){
        tail = node;
        head = node;
    }
    else{
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
}

//find a job by pid
struct Node* findJobByJobId(struct LinkedList* jobList, int id){
    if(jobList->head == NULL){
        return NULL;
    }
    else{
        struct Node* temp = jobList->head;
        while(temp != NULL){
             if( temp->data->pid == id ){
                        return temp;
                }
            temp = temp->next;
        }
    }
}

//print jobs forward
void printForward(struct LinkedList* jobList) {
        struct Node* temp = jobList->head;
        if (temp == NULL)
                printf("empty ll\n");
        while(temp != NULL) {
                printf("Job: %d\n", temp->data->pid );
                temp = temp->next;
        }
        printf("\n");
}

//print jobs forward
void printBackward(struct LinkedList* jobList) {
        struct Node* temp = jobList->tail;
        if (temp == NULL)
                printf("empty ll\n");
        while(temp != NULL) {
                printf("Job: %d\n", temp->data->pid );
                temp = temp->prev;
        }
        printf("\n");
}


int main() {


}
