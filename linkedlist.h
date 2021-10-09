//doubly linked list
#include<stdio.h>
#include<stdlib.h>

#include "jobs.h"

struct Node  {
        struct job* data; //the job struct storage
        struct Node* next;
        struct Node* prev;
};

//Global Variables
struct Node* head; //linkedList Head node 
struct Node* tail; //linkedList Tail node

//create new Node and return the pointer to it
struct Node* createNewNode(struct job* jobtoadd){
    struct Node* newJob = (struct Node *) malloc(sizeof(struct Node));
    newJob->data = jobtoadd;
    newJob->next = NULL;
    newJob->prev = NULL;
}

//Insert at the Head
void insertAtHead(struct Node* node){
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
void insertAtTail(struct Node* node){
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
struct Node* findJobByJobId(int id){
    if(head == NULL){
        return NULL;
    }
    else{
        struct Node* temp = head;
        while(temp != NULL){
             if( temp->data->pid == id ){
                        return temp;
                }
            temp = temp->next;
        }
    }
}

//print jobs forward
void printForward() {
        struct Node* temp = head;
        if (temp == NULL)
                printf("empty ll\n");
        while(temp != NULL) {
                printf("Job: %d\n", temp->data->pid );
                temp = temp->next;
        }
        printf("\n");
}

//print jobs forward
void printBackward() {
        struct Node* temp = tail;
        if (temp == NULL)
                printf("empty ll\n");
        while(temp != NULL) {
                printf("Job: %d\n", temp->data->pid );
                temp = temp->prev;
        }
        printf("\n");
}


int main() {

        /*Driver code to test the implementation*/
        head = NULL; // empty list. set head as NULL. 
        tail = NULL;                                                                                                                                                       

        // Calling an Insert and printing list both in forward as well as reverse direction.                                                                                                                

        printf("print %d\n: ", findJobByJobId( 3 ));
}
