#include "syscall.h"
#include "proc.h"
#include "console.h"

int
sys_exit()
{
    cprintf("sys_exit: in exit\n");
    exit();
    return 0;
}

int 
sys_yield()
{
    cprintf("sys_yield: in yield\n");
    yield();
    return 0;
}