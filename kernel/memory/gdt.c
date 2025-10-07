#include "kernel/memory/gdt.h"
extern void gdt_commit(gdtr_t*);


static gdt_descriptor_t __gdt[6] = {};
static gdtr_t __gdtr = {};


static void write(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_descriptor_t* entry = &__gdt[index];
    entry->base_low = base & 0xffff;
    entry->base_mid = (base >> 16) & 0xff;
    entry->base_high = (base >> 24) & 0xff;
    entry->limit_low = limit & 0xffff;
    entry->access = access;
    entry->flags = ((limit >> 12) & 0xf0) | (flags & 0xf);
}

static void init(void){
    // null
    write(0, 0, 0, 0, 0);
    // kernel code
    write(1, 0, 0xfffff, 0x9a, 0xc);
    // kernel data
    write(2, 0, 0xfffff, 0x92, 0xc);
    // user code
    write(3, 0, 0xfffff, 0xfa, 0xc);
    // user data
    write(4, 0, 0xfffff, 0xf2, 0xc);
        
    __gdtr.offset = &__gdt[0];
    __gdtr.size = sizeof(__gdt) - 1;
    
    gdt_commit(&__gdtr);
}


Gdt_t gdt = {
    .init = &init,
    .write = &write,
};
