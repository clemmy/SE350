typedef struct Node{
	void* data;
	Node* next;
}

typedef struct Queue{
	Node* first;
	Node* last;
}

Queue* initQueue();
void* first(Queue*);
void enqueue(Queue*, void*);
void dequeue(Queue*);