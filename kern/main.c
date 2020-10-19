#include <stdint.h>

#include "string.h"
#include "console.h"
#include "kalloc.h"

void
main()
{
    /*
     * Before doing anything else, we need to ensure that all
     * static/global variables start out zero.
     */

    extern char edata[], end[];

    /* TODO: Use `memset` to clear the BSS section of our program. */
    memset(edata, 0, end - edata);    
    /* TODO: Use `cprintf` to print "hello, world\n" */
    console_init();
    alloc_init();
    cprintf("Allocator: Init success.\n");
    check_free_list();
    void *p = kalloc();
    void *pgdir = kalloc();
    memset(pgdir,0,PGSIZE);
    map_region(pgdir, 0, PGSIZE, V2P(p), 0);
    memset(p,0xAC,PGSIZE);
    load_sp_el1(pgdir);
    for (int i =0; i < PGSIZE; i++){
        print(*((int *)i) == "0xAC");
    }
    while (1) ;
}