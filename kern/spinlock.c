#include "arm.h"
#include "spinlock.h"
#include "console.h"

/*void
mcs_acquire(struct mcslock *lk, struct mcslock *i)
{
    i->next = i->locked = 0;
    volatile struct mcslock *pre = __sync_lock_test_and_set(&lk->next, i);
    if(pre) {
        i->locked = 1;
        pre->next = i;
    }
    while(i->locked);
}

void
mcs_release(struct mcslock *lk, struct mcslock *i)
{
    if(i->next == 0)
        if(__sync_val_compare_and_swap(&lk->next, i, __ATOMIC_RELEASE) == i)
            return;
    while(i->next == 0) ;
    i->next->locked = 0;
}

void
read_lock(struct rwlock *lk)
{
    acquire(&lk->r);
    if(lk->cnt++==0)
        acquire(&lk->w);
    release(&lk->r);
}

void
read_unlock(struct rwlock *lk)
{
    acquire(&lk->r);
    if(--lk->cnt==0)
        release(&lk->w);
    release(&lk->r);
}

void
write_lock(struct rwlock *lk)
{
    acquire(&lk->w);
}

void
write_unlock(struct rwlock *lk)
{
    release(&lk->w);
}*/
#include "proc.h"
#include "string.h"

/*
 * Check whether this cpu is holding the lock.
 */
int
holding(struct spinlock *lk)
{
    int hold;
    hold = lk->locked && lk->cpu == thiscpu;
    return hold;
}

void
initlock(struct spinlock *lk, char *name) {
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

void
acquire(struct spinlock *lk)
{
    if (holding(lk)) {
        panic("acquire: spinlock already held\n");
    }
    while (lk->locked || __atomic_test_and_set(&lk->locked, __ATOMIC_ACQUIRE))
        ;
    lk->cpu = thiscpu;
}

void
release(struct spinlock *lk)
{
    if (!holding(lk)) {
        panic("release: not locked\n");
    }
    lk->cpu = NULL;
    __atomic_clear(&lk->locked, __ATOMIC_RELEASE);
}
