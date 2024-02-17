#include <stdint.h>

#ifndef QUEUE_H
#define QUEUE_H

#define CX_TABLE_SIZE 4096

typedef struct {
    int32_t arr[CX_TABLE_SIZE];
    int32_t front;
    int32_t rear;
    int32_t size;
    int32_t capacity;
} queue_t;

queue_t *make_queue();

void enqueue(queue_t *queue, int32_t element);

int32_t dequeue(queue_t *queue);

int32_t front(queue_t* queue);

#endif