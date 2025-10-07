#include "kernel/memory/heap.h"
#include "kernel/queue.h"


static void push(queue_t* queue, void* payload){
    queue_node_t* node = halloc.malloc(sizeof(queue_node_t));
    node->next = (void*)0;
    node->payload = payload;
    if ((void*)0 == queue->head){
        node->prev = (void*)0;
        queue->head = node;
        queue->tail = node;
    } else {
        node->prev = queue->tail;
        queue->tail->next = node;
        queue->tail = node;
    }
}

static void* pop(queue_t* queue){
    if ((void*)0 == queue->head){
        return (void*)0;
    }
    queue_node_t* head = queue->head;
    void* payload = head->payload;
    
    queue->head = head->next;
    if ((void*)0 == queue->head){
        queue->tail = (void*)0;
    } else {
        queue->head->prev = (void*)0;
    }
    halloc.free(head);
    return payload;
}

static void* peek(queue_t* queue){
    if ((void*)0 == queue->head){
        return (void*)0;
    }
    return queue->head->payload;
}


void queue_init(queue_t* queue){
    queue->head = (void*)0;
    queue->tail = (void*)0;
    queue->push = &push;
    queue->pop = &pop;
    queue->peek = &peek;
}