#include <stdint.h>
#include <stdlib.h>

#include "queue.h"

static int32_t isFull(queue_t* queue)
{
    return (queue->size == queue->capacity);
}

static int32_t isEmpty(queue_t* queue)
{
    return (queue->size == 0);
}

void enqueue(queue_t* queue, int32_t item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->arr[queue->rear] = item;
    queue->size = queue->size + 1;
}

int32_t dequeue(queue_t* queue)
{
    if (isEmpty(queue))
        return -1;
    int32_t item = queue->arr[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

int32_t front(queue_t* queue)
{
    if (isEmpty(queue))
        return -1;
    return queue->arr[queue->front];
}

queue_t* make_queue()
{
    queue_t* queue = (queue_t *) malloc(sizeof(queue_t));
    queue->capacity = CX_TABLE_SIZE;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = CX_TABLE_SIZE - 1;
    for(int32_t i = 0; i < CX_TABLE_SIZE - 1; i++) {
        enqueue(queue, i);
    }
    return queue;
}