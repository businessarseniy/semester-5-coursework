#pragma once

#include <stdint.h>


enum {
    ELF_WS_32BIT = 1,
    ELF_WS_64BIT = 2,
};

enum {
    ELF_LE = 1,
    ELF_BE = 2,
};

enum {
    ELF_TYPE_RELOCATABLE = 1,
    ELF_TYPE_EXECUTABLE = 2,
    ELF_TYPE_SHARED = 3,
    ELF_TYPE_CORE = 4,
};

enum {
    ELF_IS_NONE = 0x0,
    ELF_IS_SPARC = 0x2,
    ELF_IS_X86 = 0x3,
    // and so on...
};

enum {
    ELF_HEADER_TYPE_NULL = 0,
    ELF_HEADER_TYPE_LOAD = 1,
    ELF_HEADER_TYPE_DYNAMIC = 2,
    ELF_HEADER_TYPE_INTERP = 3,
    ELF_HEADER_TYPE_NOTE = 4,
};

enum {
    ELF_HEADER_FLAG_EXECUTABLE = 1,
    ELF_HEADER_FLAG_WRITABLE = 2,
    ELF_HEADER_FLAG_READABLE = 4,
};

enum {
    ELF_SECTION_TYPE_NOBITS = 8,
};

enum {
    ELF_SECTION_FLAG_ALLOC = 0x2,
};

typedef struct {
    uint8_t magic[4]; // 0x7F, ELF
    uint8_t word_size;
    uint8_t endianness;
    uint8_t elf_header_version;
    uint8_t os_abi;
    uint8_t __reserved[8];
    uint16_t type;
    uint16_t instruction_set;
    uint32_t elf_version;
    uint32_t program_entry_offset;
    uint32_t program_header_table_offset;
    uint32_t section_header_table_offset;
    uint32_t flags;
    uint16_t elf_header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t section_index_to_the_section_header_string_table;
}__attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t flags;
    uint32_t alignment;
}__attribute__((packed)) elf_program_header_entry_t;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
}__attribute__((packed)) elf_section_header_t;

typedef struct {
    int (*is_elf)(elf_header_t* eheader);
} Elf_t;


extern Elf_t elf;
