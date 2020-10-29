#ifndef INC_TRAP_H
#define INC_TRAP_H

#include <stdint.h>

struct trapframe {
    /* TODO: Design your own trapframe layout here. */
    uint64_t SPSR;
    uint64_t SP;
    uint64_t ESR;
    uint64_t REGs[31];
};

void trap(struct trapframe *);
void irq_init();
void irq_error();

#endif
