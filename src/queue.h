typedef struct Node Node;

struct Node{
	void* data;
	Node* next;
}

typedef struct GQueue GQueue;

struct GQueue{
	void* address;
	int numNodes;
	Node* first;
	Node* last;
}

GQueue* initQueue(void* address);
void* first(GQueue* q);
void enqueue(GQueue* q, void* ptr);
void dequeue(GQueue* q);

bool isFull(GQueue* q);
void* attach(GQueue* q1, GQueue* q2);
