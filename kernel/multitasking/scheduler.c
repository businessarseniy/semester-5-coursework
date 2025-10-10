#include "kernel/hardware/pit.h"
#include "kernel/memory/heap.h"
#include "kernel/memory/paging.h"
#include "kernel/memory/physical.h"
#include "kernel/multitasking/scheduler.h"
#include "kernel/queue.h"
#include "kernel/tty.h"


static uint32_t __pid = 1;
static process_control_block_t* __all[128] = {};
static process_control_block_t* __current = (void*)0;
static queue_t __start = {};
static queue_t __ready = {};
static queue_t __waiting = {};
static queue_t __done = {};


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

static void save_current(interrupt_frame_t* frame, process_control_block_t* current){
    if ((void*)0 == frame){
        // skip
    } else {
        current->eip = frame->eip;
        current->esp = frame->esp_original;
        // save context
        current->context.eax = frame->eax;
        current->context.ebx = frame->ebx;
        current->context.ecx = frame->ecx;
        current->context.edx = frame->edx;
        current->context.edi = frame->edi;
        current->context.esi = frame->esi;
        current->context.ebp = frame->ebp;
        current->context.eflags = frame->eflags;
    }
    switch (current->state){
        case STATE_RUNNING:
            current->state = STATE_READY;
            __ready.push(&__ready, current);
            break;
        case STATE_WAITING:
            __waiting.push(&__waiting, current);
            break;
        case STATE_DONE:
            __done.push(&__done, current);
            break;
        default:
            // unreachable?
            break;
    }
}

static void load_current(interrupt_frame_t* frame, process_control_block_t* current){
    frame->eip = current->eip;
    frame->esp_original = current->esp;
    frame->cr3 = (uint32_t)current->cr3;
    // context
    frame->eax = current->context.eax;
    frame->ebx = current->context.ebx;
    frame->ecx = current->context.ecx;
    frame->edx = current->context.edx;
    frame->edi = current->context.edi;
    frame->esi = current->context.esi;
    frame->ebp = current->context.ebp;
    frame->eflags = current->context.eflags;
    current->state = STATE_RUNNING;
}

static void schedule(interrupt_frame_t* frame){
    // save current running
    if ((void*)0 != __current){
        save_current(frame, __current);
        __current = (void*)0;
    }
    // moving all new processes into ready queue
    process_control_block_t* _s = __start.pop(&__start);
    while ((void*)0 != _s){
        tty.printf("New process %d: %x\n", _s->pid, _s->eip);
        _s->state = STATE_READY;
        __ready.push(&__ready, _s);
        
        _s = __start.pop(&__start);
    }
    // check if waiting queue was resolved
    process_control_block_t* _w = __waiting.peek(&__waiting);
    while ((void*)0 != _w && STATE_READY == _w->state){
        __ready.push(&__ready, _w);
        __waiting.pop(&__waiting);

        _w = __waiting.peek(&__waiting);
    }
    // load next ready process
    process_control_block_t* _r = __ready.pop(&__ready);
    // iterate until we find READY process,
    // moving everything other in corresponding queues
    while ((void*)0 != _r && _r->state != STATE_READY){
        switch (_r->state){
            case STATE_DONE:
                __done.push(&__done, _r);
                break;
            case STATE_WAITING:
                __waiting.push(&__waiting, _r);
                break;
            default:
                // unreachable?
        }
        _r = __ready.pop(&__ready);
    }
    if ((void*)0 == _r){
        // panic!
        tty.color(COLOR_RED);
        tty.print("Panic: scheduler empty ready queue\n");
        asm volatile("cli");
        asm volatile("hlt");
    }
    load_current(frame, _r);
    __current = _r;
    // remove everything from done processes
    process_control_block_t* _d = __done.pop(&__done);
    while ((void*)0 != _d){
        // notify parent of done process
        __all[_d->pid] = (void*)0;
        paging.destroy_directory(_d->cr3);
        halloc.free(_d);

        _d = __done.pop(&__done);
    }
}

static void init(void){
    // Tie scheduler to timer event
    pit.listen(&schedule);
    queue_init(&__start);
    queue_init(&__ready);
    queue_init(&__waiting);
    queue_init(&__done);
}

static process_control_block_t* create(uint32_t entry){
    process_control_block_t* pcb = halloc.malloc(sizeof(process_control_block_t));
    memset(pcb, sizeof(process_control_block_t), 0);
    pcb->pid = __pid++;
    __all[pcb->pid] = pcb;
    pcb->eip = entry;
    pcb->state = STATE_START;
    pcb->cr3 = paging.default_directory();
    paging.create_page(0, (void*)(((uint32_t*)pcb->cr3)[3] & ~0xfff), PRESENT | READWRITE | USER);
    pcb->stack = (void*)0xc00000;
    pcb->esp = (uint32_t)(&pcb->stack[1024]);
    pcb->context.eflags = 0x202;

    __start.push(&__start, pcb);
    return pcb;
}

static process_control_block_t* current(void){
    return __current;
}

static process_control_block_t* get(uint32_t pid){
    return __all[pid];
}


Scheduler_t scheduler = {
    .init = &init,
    .create = &create,
    .current = &current,
    .schedule = &schedule,
    .get = &get,
};
