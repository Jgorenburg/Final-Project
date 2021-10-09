//doubly linked list
#include<stdio.h>
#include<stdlib.h>

#include "job.h"

struct Node  {
        jobInfo* data; //the job struct storage
        struct Node* next;
        struct Node* prev;
};

//Global Variables
struct Node* head; //linkedList Head node 
struct Node* tail; //linkedList Tail node

//create new Node and return the pointer to it
struct Node* createNewNode(jobInfo *job){
    struct Node* newJob = (struct Node *) malloc(sizeof(struct Node));
    newJob->data = job;
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
             if( temp->data->jobid == id ){
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
                printf("Job: %d\n", temp->data->jobid );
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
                printf("Job: %d\n", temp->data->jobid );
                temp = temp->prev;
        }
        printf("\n");
}


int main() {

        /*Driver code to test the implementation*/
        head = NULL; // empty list. set head as NULL. 
        tail = NULL;                                                                                                                                                       

        jobInfo *job1 = initJob();
        job1->jobid = 1;
        struct Node *node1 = createNewNode(job1);
        jobInfo *job2= initJob();
        job2->jobid = 2;
        struct Node *node2 = createNewNode(job2);
        jobInfo *job3 = initJob();
        job3->jobid = 3;
        struct Node *node3 = createNewNode(job3);
        jobInfo *job4= initJob();
        job4->jobid = 4;
        struct Node *node4 = createNewNode(job4);



        // Calling an Insert and printing list both in forward as well as reverse direction.                                                                                                                
        insertAtTail(node1); 
        printForward();
        printBackward(); 
        insertAtTail(node2); 
        printForward();
        printBackward(); 
        insertAtHead(node3); 
        printForward();
        printBackward() ;
        insertAtHead(node4); 
        printForward();
        printBackward();
        insertAtHead(node4); 

        printf("print %d\n: ", findJobByJobId( 3 ));
}
