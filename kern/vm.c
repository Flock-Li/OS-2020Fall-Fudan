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
    /* TODO: Your code here. */
    uint64_t *current_table = pgdir;
    uint64_t *entry = NULL;
    for (int i = 0; i < 4; i++){
        offset = PTX(i, va);
        entry = current_table + offset;
        if(!(*entry & PTE_P)){
            if(alloc == 0)
                return NULL;
            char *page = kalloc();
            memset(page, 0, PGSIZE);
            *entry = V2P(*page) | PTE_P | PTE_USER | PTE_RW;
        }
        current_table =(uint64_t *) P2V(PTE_ADDR(*entry));
    }
    return entry;
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
    uint64_t va_start = ROUNDDOWN(*va);
    uint64_t va_end = ROUNDDOWN(*va + size -1);
    uint64_t *pte;
    for(;;){
        if( (pte = pgdir_walk(pgdir, va, 1)) == 0 )
            return -1;
        if( *pte & PTE_P )
            panic("remap");
        *pte = P2V(pa) | perm | PTE_P | PTE_TABLE | PTE_AF;
        if(va_start == va_end)
            break;
        va_start += PGSIZE;
        pa += PGSIZE;
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
    /* TODO: Your code here. */
    for(int i = 0; i < 512; i++){
        uint64_t pte = pgdir[i];
        if( pte & PTE_P ){
            uint64_t *child = (uint64_t *) P2V(PTE_ADDR(*entry));
            vm_free(child,level+1);
        }
    }
    kfree(pgdir);
}