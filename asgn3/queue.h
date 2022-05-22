typedef struct Queue Queue;
struct Queue{
  int front;
  int back;
  int size;
  int total;
  int content[128];

};
void init_queue(Queue* Q);
bool enQueue(Queue* Q, int val);
bool empty(Queue *Q);

int size(Queue *Q);

 bool full(Queue* Q);
 int deQueue(Queue *Q);
