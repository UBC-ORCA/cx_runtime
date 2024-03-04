#include <stdint.h>

#ifndef QUEUE_H
#define QUEUE_H

#define CX_TABLE_SIZE 1024

typedef struct {
    int32_t *arr;
    int32_t front;
    int32_t rear;
    int32_t size;
    int32_t capacity;
} queue_t;

queue_t *make_queue(int32_t capacity);

void enqueue(queue_t *queue, int32_t element);

int32_t dequeue(queue_t *queue);

int32_t front(queue_t* queue);

#endif