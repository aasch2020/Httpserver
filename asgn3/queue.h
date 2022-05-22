typedef struct Queue Queue;
struct Queue{
  int front;
  int back;
  int size;
  int total;
  int content[128];

};

 bool enQueue(Queue* Q, int val);\


int size(Queue *Q);

 bool full(Queue* Q);
 int deQueue(Queue *Q);
