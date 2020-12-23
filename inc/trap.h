#ifndef INC_TRAP_H
#define INC_TRAP_H

#include <stdint.h>

struct trapframe {
    /* TODO: Design your own trapframe layout here. */
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
    uint64_t x0, x1, x2, x3, x4;
    uint64_t x5, x6, x7, x8, x9;
    uint64_t x10, x11, x12, x13, x14;
    uint64_t x15, x16, x17, x18, x19;
    uint64_t x20, x21, x22, x23, x24;
    uint64_t x25, x26, x27, x28, x29;
    uint64_t x30;
};

void trap(struct trapframe *);
void irq_init();
void irq_error();

#endif
