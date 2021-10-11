#include "linkedlist.h"

// initialize ArrayList with assigned size
struct LinkedList* init_list() { 
    struct LinkedList* aaa = malloc(sizeof(struct LinkedList));
    aaa->head = NULL;
	aaa->tail = NULL;
    aaa->i=0;    // length is 0 at initialization
    return aaa; // return a pointer to the LinkedList that is created
}

// Destructor for ArrayList
// start from head and traverse the list to free all nodes recursively
void free_nodes(struct Node* item) {
    if (item==NULL) {return;}
    free_nodes(item->next);
    free(item->data);
	free(item);
}

void free_list(struct LinkedList *aaa) {
    struct Node* head=aaa->head;
    free_nodes(head);
    free(aaa);
}

struct Node* createNewNode(struct job* jobtoadd){
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
		printf("Job: %s\n", temp->data->input );
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
		printf("Job: %s\n", temp->data->input );
		temp = temp->prev;
	}
	printf("\n");
}

void printJobs()
{
        if (joblist->i == 0)
                return;
        struct Node* temp;
		for(temp = joblist->head; temp != NULL; temp = temp->next){
			printf("\t[%d]\t%s\t%s", temp->data->pid, temp->data->status, temp->data->input);
			if (temp->next != NULL)
            {
                printf("|\n");
            }
            else
            {
                printf("\n");
            }
		}
}



