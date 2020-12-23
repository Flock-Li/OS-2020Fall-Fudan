#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "string.h"
#include "memlayout.h"
#include "console.h"

#include "vm.h"
#include "kalloc.h"
#include "proc.h"

extern uint64_t *kpgdir;

/* 
 * Given 'pgdir', a pointer to a page directory, pgdir_walk returns
 * a pointer to the page table entry (PTE) for virtual address 'va'.
 * This requires walking the four-level page table structure.
 *
 * The relevant page table page might not exist yet.
 * If this is true, and alloc == false, then pgdir_walk returns NULL.
 * Otherwise, pgdir_walk allocates a new page table page with kalloc.
 *   - If the allocation fails, pgdir_walk returns NULL.
 *   - Otherwise, the new page is cleared, and pgdir_walk returns
 *     a pointer into the new page table page.
 */

/*static */uint64_t *
pgdir_walk(uint64_t *pgdir, const void *va, int64_t alloc)
{  //pgdir point to the start entry of given page table
    /* TODO: Your code here. */
    if((uint64_t)va >= (1L << 50)) panic("virtual memory limit exceed");
    uint64_t *pg = pgdir;
    for(int level = 0; level < 3; level++) {
        uint64_t *pte = &pg[PTX(level, va)]; // pte point to target entry in this page table
        if(*pte & PTE_P) { // check valid
            pg = (uint64_t*) P2V(PTE_ADDR(*pte));
        }
        else {
            if(alloc == 0 || (pg = (uint64_t*)kalloc()) == 0) return NULL; // not allowed to alloc or fail to alloc
            memset(pg, 0, PGSIZE); // init the new page
            *pte = V2P(pg) | PTE_TABLE | PTE_P | PTE_AF | PTE_NORMAL;
        }
    }
    return &pg[PTX(3, va)];
    /* My code ends. */
}

/*
 * Create PTEs for virtual addresses starting at va that refer to
 * physical addresses starting at pa. va and size might **NOT**
 * be page-aligned.
 * Use permission bits perm|PTE_P|PTE_TABLE|(MT_NORMAL << 2)|PTE_AF|PTE_SH for the entries.
 *
 * Hint: call pgdir_walk to get the corresponding page table entry
 */

static int
map_region(uint64_t *pgdir, void *va, uint64_t size, uint64_t pa, int64_t perm)
{
    /* TODO: Your code here. */
    char* startp = ROUNDDOWN(va, PGSIZE);
    char* endp = ROUNDDOWN(va + size - 1, PGSIZE);
    for(char* i = startp; i <= endp; i += PGSIZE, pa += PGSIZE) {
        uint64_t *pte = pgdir_walk(pgdir, i, 1);
        *pte = pa | perm | PTE_P | PTE_TABLE | (MT_NORMAL << 2) | PTE_AF | PTE_SH;
    }
    return 0;
    /* My code ends */
}

/* 
 * Free a page table.
 *
 * Hint: You need to free all existing PTEs for this pgdir.
 */

void
vm_free(uint64_t *pgdir, int level)
{
    /* TODO: Your code here. */
    if(pgdir == 0) panic("pgdir not found");
    if(level < 3) {
        for(int i = 0; i < (PGSIZE >> 3); i++) {
            uint64_t *pte = &pgdir[i];
            if(*pte & PTE_P) {
                vm_free(P2V(PTE_ADDR(*pte)), level + 1);
            }
        }
    }
    else {
        for(int i = 0; i < (PGSIZE >> 3); i++) {
            uint64_t* pte = &pgdir[i];
            if(*pte & PTE_P) {
                //cprintf("%x\n", P2V(PTE_ADDR(*pte)));
                kfree(P2V(PTE_ADDR(*pte))); // need to check valid
            }
        }
    }
    kfree((char*)pgdir);
    /* My code ends */
}

/* Get a new page table */
uint64_t *
pgdir_init()
{
    /* TODO: Your code here. */
    uint64_t *pgdir;
    if((pgdir = (uint64_t)kalloc()) == 0)
        return 0;
    memset(pgdir, 0, PGSIZE);
    return pgdir;
}

/* 
 * Load binary code into address 0 of pgdir.
 * sz must be less than a page.
 * The page table entry should be set with
 * additional PTE_USER|PTE_RW|PTE_PAGE permission
 */
void
uvm_init(uint64_t *pgdir, char *binary, int sz)
{
    /* TODO: Your code here. */
    char *mem;
    if(sz >= PGSIZE)
        panic("uvm_init: size is larger than a page");
    mem = kalloc();
    memset(mem, 0, PGSIZE);
    map_region(pgdir, 0, PGSIZE, V2P(mem), PTE_USER|PTE_RW|PTE_PAGE);
    memmove(mem, binary, sz);
}

/*
 * switch to the process's own page table for execution of it
 */
void
uvm_switch(struct proc *p)
{
    /* TODO: Your code here. */
    if(p == 0)
        panic("uvm_switch: no proc");
    if(p->pgdir == 0)
        panic("uvm_switch: no pgdir in proc");
    lttbr0(V2P(p->pgdir));
}