#include "kernel/memory/paging.h"
#include "kernel/memory/physical.h"


static void memset(void* dst, int size, int value){
    uint8_t* p_dst = dst;
    for (int i = 0; i < size; ++i){
        p_dst[i] = (uint8_t)value;
    }
}

static void* create_directory(void){
    // Creates empty page directory
    // Returns physical directory allocation address
    void* directory = palloc.malloc(-1);
    memset(directory, 4096, 0);
    return directory;
}

static void* create_table(int index, void* directory, int flags){
    // Creates empty page table and assigns it to a page directory with flags
    // Returns physical table allocation address
    void* table = palloc.malloc(-1);
    memset(table, 4096, 0);
    ((uint32_t*)directory)[index] = (uint32_t)table | (flags & 0xfff);
    return table;
}

static void create_page(int index, void* table, int flags){
    // Allocates page into page table with flags
    uint32_t* page = palloc.malloc(-1);
    memset(page, 4096, 0);
    ((uint32_t*)table)[index] = (uint32_t)page | (flags & 0xfff);
}

static void destroy_page(int index, void* table){
    // Clears table entry based on index and frees physical allocation of page
    void* page = (void*)(((uint32_t*)table)[index] & ~(0xfff));
    palloc.free(page);
    ((uint32_t*)table)[index] = 0;
}

static void destroy_table(int index, void* directory){
    // Clears table and deallocates pages + table
    void* table = (void*)(((uint32_t*)directory)[index] & ~(0xfff));
    for (int i = 0; i < 1024; ++i){
        destroy_page(i, table);
    }
    palloc.free(table);
    ((uint32_t*)directory)[index] = 0;
}

static void destroy_directory(void* directory){
    // Clears directory and deallocates tables + directory
    for (int i = 0; i < 1024; ++i){
        destroy_table(i, directory);
    }
    palloc.free(directory);
}

static void* default_directory(void){
    void* directory = create_directory();
    // First 4MiB of RAM -- kernel itself
    uint32_t* table0 = create_table(0, directory, PRESENT | READWRITE);
    for (int i = 0; i < 1024; ++i){
        table0[i] = (0x1000 * i) | PRESENT | READWRITE;
    }
    // 4MiB..8MiB of RAM -- kernel page allocations
    uint32_t* table1 = create_table(1, directory, PRESENT | READWRITE);
    for (int i = 0; i < 1024; ++i){
        table1[i] = (0x400000 + 0x1000 * i) | PRESENT | READWRITE;
    }
    // 8MiB..12MiB of RAM -- kernel heap allocations
    uint32_t* table2 = create_table(2, directory, PRESENT | READWRITE);
    for (int i = 0; i < 1024; ++i){
        table2[i] = (0x800000 + 0x1000 * i) | PRESENT | READWRITE;
    }

    return directory;
}

static void init(void){
    void* directory = default_directory();
    asm volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        : "r"(directory)
        : "eax"
    );
}

Paging_t paging = {
    .init = &init,
    .default_directory = &default_directory,
    .create_directory = &create_directory,
    .destroy_directory = &destroy_directory,
    .create_table = &create_table,
    .destroy_table = &destroy_table,
    .create_page = &create_page,
    .destroy_page = &destroy_page,
};
