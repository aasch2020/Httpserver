#include <stdbool.h>
#include "queue.h"

void init_queue(Queue *Q) {
    Q->front = Q->back = -1;
    Q->size = 0;
    Q->total = 128;
}

int size(Queue *Q) {
    return Q->size;
}

bool empty(Queue *Q) {
    return ((Q->front == -1));
}
bool full(Queue *Q) {
    return (
        (Q->front == 0 && Q->back == Q->total - 1) || (Q->back == (Q->front - 1) % (Q->total = 1)));
}

bool enQueue(Queue *Q, int val) {
    if (full(Q)) {

        return false;
    }
    if (Q->front == -1) {
        Q->front = Q->back = 0;
        Q->content[Q->back] = val;
        Q->size++;
        return true;
    }
    if (Q->back == Q->total - 1 && Q->front != 0) {
        Q->back = 0;
        Q->content[Q->back] = val;
        Q->size++;
        return true;
    } else {
        Q->back++;
        Q->size++;
        Q->content[Q->back] = val;
        return true;
    }
}

int deQueue(Queue *Q) {
    if (Q->front == -1 || Q->size == 0) {
        return -1;
    }
    int contents = Q->content[Q->front];
    if (Q->back == Q->front) {
        Q->front = Q->back = -1;
    } else if (Q->front == Q->total - 1) {

        Q->front = 0;
    } else {

        Q->front++;
    }

    Q->size--;
    return contents;
}
