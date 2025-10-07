#pragma once


// #include <stdint.h>


typedef struct __queue_node {
    void* payload;
    struct __queue_node* next;
    struct __queue_node* prev;
} queue_node_t;

typedef struct __queue{
    queue_node_t* head;
    queue_node_t* tail;
    void (*push)(struct __queue* queue, void* payload);
    void* (*pop)(struct __queue* queue);
    void* (*peek)(struct __queue* queue);
} queue_t;


void queue_init(queue_t* queue);
