#include "kernel/filesystem/vfs.h"
#include "kernel/interrupt/syscall.h"
#include "kernel/memory/heap.h"
#include "kernel/memory/paging.h"
#include "kernel/multitasking/elf.h"
#include "kernel/multitasking/scheduler.h"
#include "kernel/tty.h"


typedef struct __stat stat_t;
typedef /* off_t */ int off_t;
typedef /* _ssize_t */ int _ssize_t;
typedef /* size_t */ int size_t;
typedef /* ptrdiff_t */ int ptrdiff_t;
typedef /* clock_t */ int clock_t;
typedef struct __tms tms_t;



static void memcpy(void* dst, void* src, int size){
    uint8_t* p_dst = dst;
    uint8_t* p_src = src;
    for (int i = 0; i < size; ++i){
        p_dst[i] = p_src[i];
    }
}

static void memset(void* dst, int size, int val){
    uint8_t* p_dst = dst;
    for (int i = 0; i < size; ++i){
        p_dst[i] = (uint8_t)val;
    }
}

static void notify_parent_complete(process_control_block_t* process){
    if ((void*)0 == process->parent) return;
    for (int i = 0; i < 32; ++i){
        if (process->parent->child[i] != process->pid) continue;
        // found bit position
        switch (process->parent->state){
            case STATE_WAITING:
                process->parent->context.eax = process->pid;
                process->parent->child[i] = 0;
                process->parent->child_bitmap &= ~(1U << i);
                process->parent->state = STATE_READY;
                break;
            case STATE_READY:
                process->parent->child_completed_bitmap |= (1U << i);
                break;
            default:
                // unreachable?
                break;
        }
        return;
    }
}

static void notify_parent_create(process_control_block_t* process){
    // make sure process parent is known
    for (int i = 0; i < 32; ++i){
        if (0 == (process->parent->child_bitmap & (1U << i))){
            process->parent->child_bitmap |= (1U << i);
            process->parent->child[i] = process->pid;
            break;
        }
    }
}

static int close(interrupt_frame_t* frame, int fd){
    int res;
    switch (fd){
        case 0: // STDIN
        case 1: // STDOUT
        case 2: // STDERR
            res = 0;
            break;
        default:
            res = vfs.close(fd);
            break;
    }
    return res;
}
static int execve(interrupt_frame_t* frame, char* name, char* argv[], char* env[]){
    process_control_block_t* current = scheduler.current();
    if (0xffffffff == current->child_bitmap){
        return -1;
    }
    // no flags and mode for now
    int fd = vfs.open(name, -1, -1);
    // not found executable
    if (fd == -1){
        return -1;
    }
    
    elf_header_t* eheader = halloc.malloc(sizeof(elf_header_t));
    vfs.read(fd, eheader, sizeof(elf_header_t));

    if (!elf.is_elf(eheader)){
        halloc.free(eheader);
        vfs.close(fd);
        return -1;
    }
    
    // elf x86 executable
    int pheader_size = eheader->program_header_entry_count * eheader->program_header_entry_size;
    elf_program_header_entry_t* pheader = halloc.malloc(pheader_size);
    vfs.lseek(fd, eheader->program_header_table_offset, -1);
    vfs.read(fd, pheader, pheader_size);
    
    process_control_block_t* new = scheduler.create(eheader->program_entry_offset);
    uint32_t* directory = new->cr3;
    for (int i = 0; i < eheader->program_header_entry_count; ++i){
        if (pheader[i].type != ELF_HEADER_TYPE_LOAD) continue;
        // only PT_LOAD for now
        int paging_table = pheader[i].p_vaddr / 0x400000;
        int page_index = (pheader[i].p_vaddr % 0x400000) / 0x1000;
        uint32_t table = directory[paging_table] & ~0xfff;
        int num_pages = (pheader[i].p_filesz + 0x1000 - 1) / 0x1000;
        int size = pheader[i].p_filesz;
        for (int j = 0; j < num_pages; ++j){
            void* page = paging.create_page(
                page_index + j,
                (void*)table,
                PRESENT | USER | (pheader[i].flags & ELF_HEADER_FLAG_WRITABLE ? READWRITE : 0)
            );
            memset(page, 4096, 0);
            vfs.lseek(fd, pheader[i].p_offset + j * 0x1000, -1);
            if (size > 0x1000){
                vfs.read(fd, page, 0x1000);
                size -= 0x1000;
            } else {
                vfs.read(fd, page, size);
            }
        }
    }

    int sheader_size = eheader->section_header_entry_size * eheader->section_header_entry_count;
    elf_section_header_t* sheader = halloc.malloc(sheader_size);
    vfs.lseek(fd, eheader->section_header_table_offset, -1);
    vfs.read(fd, sheader, sheader_size);

    for (int i = 0; i < eheader->section_header_entry_count; ++i){
        elf_section_header_t* section = &sheader[i];
        if (ELF_SECTION_TYPE_NOBITS == section->sh_type){
            if (!(section->sh_size)) continue;

            if (ELF_SECTION_FLAG_ALLOC & section->sh_flags){
                int paging_table = section->sh_addr / 0x400000;
                int page_index = (section->sh_addr % 0x400000) / 0x1000;
                uint32_t table = directory[paging_table] & ~0xfff;
                void* page = paging.create_page(page_index, (uint32_t*)table, PRESENT | READWRITE | USER);
                memset(page, 0x1000, 0);
            }
        }
    }

    new->parent = current;
    notify_parent_create(new);
    current->state = STATE_WAITING;

    // argv
    // env
    new->stack[1023] = 0;
    --new->esp;
    new->stack[1022] = 0;
    --new->esp;
    
    halloc.free(pheader);
    halloc.free(eheader);
    vfs.close(fd);
    return new->pid;
}
static int fork(interrupt_frame_t* frame){
    process_control_block_t* current = scheduler.current();
    if (0xffffffff == current->child_bitmap){
        return -1;
    }
    process_control_block_t* forked = scheduler.create(frame->eip);
    forked->parent = current;
    notify_parent_create(forked);
    forked->context = current->context;
    forked->esp = current->esp - (uint32_t)current->stack + (uint32_t)forked->stack;
    memcpy(forked->stack, current->stack, 4096);
    // add forked to child processes of current
    // add current as parent for forked
    // maybe copy page directory?
    // maybe copy file descriptors?
    return forked->pid;
}
static int wait(interrupt_frame_t* frame, int* status){
    process_control_block_t* current = scheduler.current();
    if (0 == current->child_bitmap){
        // no childs
        return -1;
    }
    if (0 == current->child_completed_bitmap){
        // no completed childs, wait state
        // don't forget to return pid of killed/exited process into parent's eax
        // with changing state to READY if no childs remaining
        // also schedule() if current process state is waiting after syscall
        current->state = STATE_WAITING;
        return 0;
    }
    // in case that child returned before wait call?
    for (int i = 0; i < 32; ++i){
        if (0 == (current->child_completed_bitmap & (1U << i))) continue;
        // found completed child
        current->child_bitmap &= ~(1U << i);
        current->child_completed_bitmap &= ~(1U << i);
        uint32_t completed_pid = current->child[i];
        current->child[i] = 0;
        return completed_pid;
    }
    // unreachable?
}
static int fstat(interrupt_frame_t* frame, int fd, stat_t* pstat){
    return 0;
}
static int link(interrupt_frame_t* frame, char* old, char* new){
    return 0;
}
static off_t lseek(interrupt_frame_t* frame, int fd, off_t pos, int whence){
    off_t res = 0;
    switch (fd){
        case 0: // STDIN
        case 1: // STDOUT
        case 2: // STDERR
            res = 0;
            break;
        default:
            res = vfs.lseek(fd, pos, whence);
            break;
    }
    return res;
}
static int open(interrupt_frame_t* frame, char* file, int flags, int mode){
    return vfs.open(file, flags, mode);
}
static _ssize_t read(interrupt_frame_t* frame, int fd, void* buf, size_t cnt){
    _ssize_t res = 0;
    switch (fd){
        case 0: // STDIN
            // not implemented
            res = 0;
            break;
        case 1: // STDOUT
        case 2: // STDERR
            res = 0;
            break;
        default: // file
            res = vfs.read(fd, buf, cnt);
            break;
    }
    return res;
}
static void* sbrk(interrupt_frame_t* frame, ptrdiff_t incr){
    process_control_block_t* current = scheduler.current();
    uint32_t heap_end = current->heap_end;
    if (0 == incr){
        return (void*)heap_end;
    }
    int page_table = heap_end / 0x400000;
    int page_index = (heap_end % 0x400000) / 0x1000;

    uint32_t* table = (uint32_t*)(current->cr3[page_table] & ~0xfff);
    // TODO: boundary table process
    int num = (incr + 0x1000 - 1) / 0x1000;
    for (int i = 0; i < num; ++i){
        paging.create_page(page_index + i, table, PRESENT | READWRITE | USER);
    }
    current->heap_end = heap_end + 0x1000 * num;
    return (void*)heap_end;
}
static int kill(interrupt_frame_t* frame, int pid, int sig){
    // no check for relation with process with pid
    process_control_block_t* process = scheduler.get(pid);
    if ((void*)0 == process) return -1;
    process->signal = sig;
    process->state = STATE_DONE;
    notify_parent_complete(process);
    return 0;
}
static int getpid(interrupt_frame_t* frame){
    return scheduler.current()->pid;
}
static int stat(interrupt_frame_t* frame, char* file, stat_t* pstat){
    return 0;
}
static clock_t times(interrupt_frame_t* frame, tms_t* ptms){
    return 0;
}
static int unlink(interrupt_frame_t* frame, char* file){
    return 0;
}
static _ssize_t write(interrupt_frame_t* frame, int fd, void* buf, size_t cnt){
    _ssize_t res = 0;
    switch (fd){
        case 0: // STDIN
            res = 0;
            break;
        case 1: // STDOUT
        case 2: // STDERR
            for (int i = 0; i < cnt; ++i) tty.putchar(((char*)buf)[i]);
            res = cnt;
            break;
        default: // file
            res = 0;
            break;
    }
    return res;
}
static void exit(interrupt_frame_t* frame, int code){
    process_control_block_t* current = scheduler.current();
    current->state = STATE_DONE;
    current->exit_code = code;
    notify_parent_complete(current);
}
static int isatty(interrupt_frame_t* frame, int file){
    int res;
    switch (file){
        case 0: // STDIN
        case 1: // STDOUT
        case 2: // STDERR
            res = 1;
            break;
        default:
            res = 0;
            break;
    }
    return res;
}


static void handler(interrupt_frame_t* frame){
    tty.printf("Syscall %d happened!\n", frame->eax);
    tty.printf("    Stack @%x, %x:%x\n", frame->esp_original, frame->cs, frame->eip);
    // syscall selector
    int eax = frame->eax;
    // arguments
    int arg0 = frame->ebx;
    int arg1 = frame->ecx;
    int arg2 = frame->edx;

    switch (eax){
        case SYSCALL_CLOSE:
            frame->eax = close(frame, arg0);
            break;
        case SYSCALL_EXECVE:
            frame->eax = execve(frame, (char*)arg0, (char**)arg1, (char**)arg2);
            scheduler.schedule(frame);
            break;
        case SYSCALL_FORK:
            frame->eax = fork(frame);
            break;
        case SYSCALL_FSTAT:
            frame->eax = fstat(frame, arg0, (stat_t*)arg1);
            break;
        case SYSCALL_GETPID:
            frame->eax = getpid(frame);
            break;
        case SYSCALL_KILL:
            frame->eax = kill(frame, arg0, arg1);
            if (arg0 == scheduler.current()->pid){
                // process killing itself
                scheduler.schedule(frame);
            }
            break;
        case SYSCALL_LINK:
            frame->eax = link(frame, (char*)arg0, (char*)arg1);
            break;
        case SYSCALL_LSEEK:
            frame->eax = lseek(frame, arg0, (off_t)arg1, arg2);
            break;
        case SYSCALL_OPEN:
            frame->eax = open(frame, (char*)arg0, arg1, arg2);
            break;
        case SYSCALL_READ:
            frame->eax = read(frame, arg0, (void*)arg1, (size_t)arg2);
            break;
        case SYSCALL_SBRK:
            frame->eax = (uint32_t)sbrk(frame, (ptrdiff_t)arg0);
            break;
        case SYSCALL_STAT:
            frame->eax = stat(frame, (char*)arg0, (stat_t*)arg1);
            break;
        case SYSCALL_TIMES:
            frame->eax = times(frame, (tms_t*)arg0);
            break;
        case SYSCALL_UNLINK:
            frame->eax = unlink(frame, (char*)arg0);
            break;
        case SYSCALL_WAIT:
            frame->eax = wait(frame, (int*)arg0);
            if (scheduler.current()->state == STATE_WAITING){
                // process is waiting
                scheduler.schedule(frame);
            }
            break;
        case SYSCALL_WRITE:
            frame->eax = write(frame, arg0, (void*)arg1, (size_t)arg2);
            break;
        case SYSCALL_EXIT:
            exit(frame, arg0);
            scheduler.schedule(frame);
            break;
        case SYSCALL_ISATTY:
            frame->eax = isatty(frame, arg0);
            break;
    }
}


Syscall_t syscall = {
    .handler = &handler,
};
