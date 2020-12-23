#include <stdint.h>

#include "arm.h"
#include "string.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "timer.h"
#include "spinlock.h"

static int ex = -1;
static struct spinlock lk = {0, "main", 0};
#include "proc.h"

struct cpu cpus[NCPU];

void
main()
{
    /*
     * Before doing anything else, we need to ensure that all
     * static/global variables start out zero.
     */

    extern char edata[], end[], vectors[];
    static int first = -1;
    acquire(&lk);
    if(first == -1) first = cpuid();
    /*
     * Determine which functions in main can only be
     * called once, and use lock to guarantee this.
     */
    /* TODO: Your code here. */

    cprintf("main: [CPU%d] is init kernel\n", cpuid());

    /* TODO: Use `memset` to clear the BSS section of our program. */
    if(first == cpuid())
    {
        memset(edata, 0, end - edata);
        console_init();
        alloc_init();
        cprintf("main: allocator init success.\n");
        check_free_list();

        irq_init();
        proc_init();
        user_init();
    }
    release(&lk);

    lvbar(vectors);
    timer_init();

    cprintf("main: [CPU%d] Init success.\n", cpuid());
    scheduler();
    while (1) ;
}
