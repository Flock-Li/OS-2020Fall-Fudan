#include <stdint.h>

#include "arm.h"
#include "string.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "timer.h"
#include "spinlock.h"
#include "proc.h"

struct cpu cpus[NCPU];

static struct spinlock conslock;
static int cpu = -1;

void
main()
{
    /*
     * Before doing anything else, we need to ensure that all
     * static/global variables start out zero.
     */

    extern char edata[], end[], vectors[];

    /*
     * Determine which functions in main can only be
     * called once, and use lock to guarantee this.
     */
    /* TODO: Your code here. */
<<<<<<< HEAD

    cprintf("main: [CPU%d] is init kernel\n", cpuid());

    /* TODO: Use `memset` to clear the BSS section of our program. */
    memset(edata, 0, end - edata);
    console_init();
    alloc_init();
    cprintf("main: allocator init success.\n");
    check_free_list();

    irq_init();
    proc_init();
    user_init();
=======
    
    acquire(&conslock);
    if (cpu < 0){
        cpu = cpuid();
        memset(edata, 0, end - edata);    
        /* TODO: Use `cprintf` to print "hello, world\n" */
        console_init();
        alloc_init();
        cprintf("Allocator: Init success.\n");
        check_free_list();
        cprintf("finish check");
        irq_init();
    }
    /* TODO: Use `memset` to clear the BSS section of our program. */
    release(&conslock);
>>>>>>> Finish lab4

    lvbar(vectors);
    timer_init();

    cprintf("main: [CPU%d] Init success.\n", cpuid());
    scheduler();
    while (1) ;
}
