#include "kernel/multitasking/elf.h"


static int is_elf(elf_header_t* eheader){
    return
        eheader->magic[0] == '\x7f' &&
        eheader->magic[1] == 'E' &&
        eheader->magic[2] == 'L' &&
        eheader->magic[3] == 'F' &&
        eheader->instruction_set == ELF_IS_X86 &&
        eheader->type == ELF_TYPE_EXECUTABLE
    ;
}


Elf_t elf = {
    .is_elf = &is_elf,
};
