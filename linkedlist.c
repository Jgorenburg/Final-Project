#include "linkedlist.h"


struct Node* createNewNode(struct LinkedList* jobList, struct job* jobtoadd){
	struct Node* newJob = (struct Node *) malloc(sizeof(struct Node));
	newJob->data = jobtoadd;
	newJob->next = NULL;
	newJob->prev = NULL;
	return newJob;
}

//Insert at the Head
void insertAtHead(struct LinkedList* jobList, struct Node* node){
	if(jobList->head == NULL){
		jobList->head = node;
		jobList->tail = node;
	}
	else{
		jobList->head->prev = node;
		node->next = jobList->head;
		jobList->head = node;
	}
}

//insert at tail of the doubly linked List
void insertAtTail(struct LinkedList* jobList, struct Node* node){
	if(jobList->tail == NULL){
		jobList->tail = node;
		jobList->head = node;
	}
	else{
		jobList->tail->next = node;
		node->prev = jobList->tail;
		jobList->tail = node;
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

		// if no job with given id
		return NULL;
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


