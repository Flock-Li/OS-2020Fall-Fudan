#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "string.h"
#include "memlayout.h"
#include "console.h"

#include "vm.h"
#include "kalloc.h"

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

static uint64_t *
pgdir_walk(uint64_t *pgdir, const void *va, int64_t alloc)
{
    uint64_t *current_table = pgdir;
    uint64_t *entry = NULL;
    for (int i = 0; i < 4; i++){
        uint64_t offset = PTX(i, va);
        entry = current_table + offset;
        if(!(*entry & PTE_P)){
            if(alloc == 0)
                return NULL;
            char *page = kalloc();
            memset(page, 0, PGSIZE);
            *entry = V2P(page) |  PTE_USER | PTE_RW;
        }
        current_table =(uint64_t *) P2V(PTE_ADDR(*entry));
    }
    return entry;
    /* TODO: Your code here. */
}

/*
 * Create PTEs for virtual addresses starting at va that refer to
 * physical addresses starting at pa. va and size might **NOT**
 * be page-aligned.
 * Use permission bits perm|PTE_P|PTE_TABLE|PTE_AF for the entries.
 *
 * Hint: call pgdir_walk to get the corresponding page table entry
 */

static int
map_region(uint64_t *pgdir, void *va, uint64_t size, uint64_t pa, int64_t perm)
{
    /* TODO: Your code here. */
    uint64_t va_start = ROUNDDOWN((char *)va,PGSIZE);
    uint64_t va_end = ROUNDDOWN((char *)va + size - 1,PGSIZE);
    uint64_t pa_start = ROUNDDOWN(pa,PGSIZE);
    uint64_t *pte;
    for(;;){
        if( (pte = pgdir_walk(pgdir, (void *)va_start, 1)) == 0 )
            return -1;
        if( *pte & PTE_P )
            panic("remap");
        *pte = pa_start | perm | PTE_P | PTE_TABLE | PTE_AF;
        if(va_start == va_end)
            break;
        va_start += PGSIZE;
        pa_start += PGSIZE;
    }
    return 0;
}

/* 
 * Free a page table.
 *
 * Hint: You need to free all existing PTEs for this pgdir.
 */

void
vm_free(uint64_t *pgdir, int level)
{
    for(int i = 0; i < 512; i++){
        uint64_t pte = pgdir[i];
        if( pte & PTE_P ){
            uint64_t *child = (uint64_t *) P2V(PTE_ADDR(pte));
            vm_free(child,level+1);
        }
    }
    kfree((char*)pgdir);
    /* TODO: Your code here. */
}