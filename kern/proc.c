#include "proc.h"
#include "spinlock.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "string.h"
#include "vm.h"
#include "mmu.h"

struct {
    struct proc proc[NPROC];
    struct spinlock lock;
    
} ptable;

static struct proc *initproc;

int nextpid = 1;
void forkret();
extern void trapret();
void swtch(struct context **, struct context *);

/*
 * Initialize the spinlock for ptable to serialize the access to ptable
 */


void
proc_init()
{
    initlock(&ptable.lock, "ptable");
}

/*
 * Look through the process table for an UNUSED proc.
 * If found, change state to EMBRYO and initialize
 * state (allocate stack, clear trapframe, set context for switch...)
 * required to run in the kernel. Otherwise return 0.
 */

static struct proc *
proc_alloc()
{
    struct proc *p;
    char *sp;
    /* TODO: Your code here. */
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->state == UNUSED) {
            goto found;
        } 
    }
    release(&ptable.lock);
    return 0;

found:
    p->pid = nextpid++;
    p->state = EMBRYO;
    release(&ptable.lock);

    //kernel stack
    if ((p->kstack = kalloc()) == 0){
        p->state = UNUSED;
        return 0;
    }
    sp = p->kstack + KSTACKSIZE;

    sp -= sizeof(*(p->tf));
    p->tf = (struct trapframe *)sp;

    sp -= 8;
    *(uint64_t *)sp = (uint64_t)trapret;

    sp -= 8;
    *(uint64_t *)sp = (uint64_t)p->kstack + KSTACKSIZE;
    
    sp -= sizeof(*(p->context));
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof(*(p->context)));

    p->context->x30 = (uint64_t)forkret + 8;

    return p;
}

/*
 * Set up first user process(Only used once).
 * Set trapframe for the new process to run
 * from the beginning of the user process determined 
 * by uvm_init
 */
char* safestrcpy(char *s, const char *t, int n)
{
    char *os;

    os = s;

    if(n <= 0) {
        return os;
    }

    while(--n > 0 && (*s++ = *t++) != 0)
        ;

    *s = 0;
    return os;
}
void
user_init()
{
    struct proc *p;
    /* for why our symbols differ from xv6, please refer https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean */
    extern char _binary_obj_user_initcode_start[], _binary_obj_user_initcode_size[];
    
    /* TODO: Your code here. */
    p = proc_alloc();
    initproc = p;

    if((p->pgdir = pgdir_init()) == 0){
        panic("userinit: out of memory?");
    }

    uvm_init(p->pgdir, _binary_obj_user_initcode_start, (int)_binary_obj_user_initcode_size);
    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*(p->tf)));
    p->tf->pc = 0;
    p->tf->sp = PGSIZE;

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->state = RUNNABLE;

}

/*
 * Per-CPU process scheduler
 * Each CPU calls scheduler() after setting itself up.
 * Scheduler never returns.  It loops, doing:
 *  - choose a process to run
 *  - swtch to start running that process
 *  - eventually that process transfers control
 *        via swtch back to the scheduler.
 */
void
scheduler()
{
    struct proc *p;
    struct cpu *c = thiscpu;
    
    for (;;) {
        /* Loop over process table looking for process to run. */
        /* TODO: Your code here. */
        acquire(&ptable.lock);
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->state == RUNNABLE) {
                // Switch to chosen process.  It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                
                c->proc = p;
                uvm_switch(p);

                p->state = RUNNING;
                
                swtch(&c->scheduler, p->context);

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
            }
        }
        release(&ptable.lock);
    }
}

/*
 * Enter scheduler.  Must hold only ptable.lock
 */

void
sched()
{
    /* TODO: Your code here. */
    
    struct cpu *c = thiscpu;
    struct proc *p = c->proc;


    if(!(&ptable.lock)->locked) {
        panic("sched ptable.lock");
    }
    if(p->state == RUNNING)
        panic("sched running");

    swtch(&p->context, c->scheduler);
}

/*
 * A fork child will first swtch here, and then "return" to user space.
 */
void
forkret()
{
    /* TODO: Your code here. */
    static int first = 1;

    release(&ptable.lock);

    if (first){
        first = 0;
    }
}
/*
 * Exit the current process.  Does not return.
 * An exited process remains in the zombie state
 * until its parent calls wait() to find out it exited.
 */

void
wakeup(void *chan)
{
    struct proc *p;
    struct cpu *c = thiscpu;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p != c->proc){
            if(p->state == SLEEPING && p->chan == chan) {
                p->state = RUNNABLE;
            }
        }
    }
}

void
exit()
{
    struct proc *p = thiscpu->proc;
    /* TODO: Your code here. */

    if(p == initproc)
        panic("init exiting");

    acquire(&ptable.lock);
    wakeup(p->parent);
    struct proc *pp;
    for(pp = ptable.proc; pp < &ptable.proc[NPROC]; pp++){
        if(pp->parent == p){
            pp->parent = initproc;
            if(pp->state == ZOMBIE){
                wakeup(initproc);
            }
            
        }
    }
    
    p->state = ZOMBIE;
    
    sched();
    panic("zombie exit");

}

// Give up the CPU for one scheduling round.
void yield(void)
{
    acquire(&ptable.lock);  //DOC: yieldlock
    thiscpu->proc->state = RUNNABLE;
    sched();
    release(&ptable.lock);
}