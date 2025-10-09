#include "kernel/memory/heap.h"
#include "kernel/tty.h"


#define HEAP_START_ADRESS ((void*)0x800000)
#define HEAP_END_ADRESS ((void*)0xc00000)
#define HEAP_BLOCK_MIN_SIZE 256


static heap_block_header_t* initial_header = HEAP_START_ADRESS;


static void* heap_chain_fit(heap_block_header_t* header, int size){
    if (0 == header->available) return (void*)0;
    if (header->size >= size){
        return header;
    }
    return heap_chain_fit(header->next, size - header->size);
}

static heap_block_header_t* heap_join_chain(heap_block_header_t* first, heap_block_header_t* last){
    first->available = 0;
    first->next = last->next;
    first->size = (void*)last->next - (void*)first - sizeof(heap_block_header_t);
    return first;
}

static void* malloc(int size){
    heap_block_header_t* header = initial_header;
    heap_block_header_t* last_header = (void*)0;
    do {
        last_header = heap_chain_fit(header, size);
        if ((void*)0 != last_header){
            heap_block_header_t* result = heap_join_chain(header, last_header);
            initial_header = result->next;
            return (void*)result + sizeof(heap_block_header_t);
        }
        while (header->available == 1){
            header = header->next;
        }
        header = header->next;
    } while((void*)0 == last_header);
}

static void free(void* ptr){
    if (ptr < HEAP_START_ADRESS || ptr > HEAP_END_ADRESS) return;
    heap_block_header_t* header = ptr - sizeof(heap_block_header_t);
    header->available = 1;
    uint32_t freed_blocks = 1;
    while (header->size > 2*HEAP_BLOCK_MIN_SIZE){
        heap_block_header_t* next_header = (void*)header + HEAP_BLOCK_MIN_SIZE;
        next_header->size = header->size - HEAP_BLOCK_MIN_SIZE;
        next_header->next = header->next;
        header->next = next_header;
        header->size = HEAP_BLOCK_MIN_SIZE - sizeof(heap_block_header_t);
        header = header->next;
        ++freed_blocks;
    }
}

static void init(void){
    // initial divide into min sized blocks
    for (void* h = HEAP_START_ADRESS; h < HEAP_END_ADRESS; h += HEAP_BLOCK_MIN_SIZE){
        heap_block_header_t* header = h;
        header->available = 1;
        void* next = (void*)header + HEAP_BLOCK_MIN_SIZE;
        if (next >= HEAP_END_ADRESS){
            next = HEAP_START_ADRESS;
            header->available = 0;
        }
        header->next = next;
        header->size = HEAP_BLOCK_MIN_SIZE - sizeof(heap_block_header_t);
    }
    void* a = malloc(10);
    void* b = malloc(10);
    tty.printf("Allocated: %x, %x\n", a, b);
    free(b);
    void* c = malloc(10);
    tty.printf("New alloc: %x\n", c);
    free(a);
    free(c);
}


HAlloc_t halloc = {
    .init = &init,
    .malloc = &malloc,
    .free = &free,
};
