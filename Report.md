# Lab4 Report

## 习题一

_start中：
```
adr x0, mp_start 将mp_start的地址保存在x0寄存器中
mov x1， #1  赋值x1寄存器为1
str x0， [x9, x1, lsl #3] 将x0寄存器的值保存在x9+x1<<3
```
通过_start，每个CPU都被设置为从mp_start处开始执行。

通过mp_start，最终都跳到el1处执行：
```
and x9, x9, #0xc Get current exception level by CurrentEL[3:2]
beq el2 跳转到el2
bgt el3 跳转到el3
```
```
adr x9, el2
msr elr_el3, x9
eret
el3出来跳转到el2

adr x9, el1
msr elr_el2, x9
eret
el2出来跳转到el1
```
而在el1中进行了页表初始化，栈指针的计算和初始化
```
adr     x9, kpgdir 将kpgdir地址保存在x9寄存器中

/*  Translation Table Base Register
Higher and lower half map to same physical memory region. */
msr ttbr0_el1, x9
msr ttbr1_el1, x9

/* sp = _start - cpuid() * PGSIZE; */
mov x10, #PGSIZE
mul x10, x10, x0
ldr x9, =_start
sub x9, x9, x10
msr spsel, #1
mov sp, x9
```
## 习题二
对于自旋锁，如果没有关闭内核中断，可能产生递归使用锁的情况，即在没有释放锁的情况下再次申请锁，会产生死锁现象。

由于自旋锁不支持递归，所以要支持内核中断，需要使用别的同步机制.

比如使用Semaphores，可以给信号量一个初始量S（>=0），一个申请锁的操作P导致S-1；一个是放锁的操作V导致S+1。也就是说最多可递归申请S个锁，即可以支持一定的内核中断。


## 习题三

依据：
* 全局初始化可以使用一个内核进行，而本地的初始化所有内核都需要执行。
* 由于所有的CPU拥有各自的寄存器（包括通用寄存器和系统寄存器），但共享相同的内存和MMIO。
所以认为初始化内存的部分是由某个内核执行，而寄存器的初始化所有核都要设置

全局初始化：
* memset： 清除BSS段
* alloc_init： 进行UART的初始化，并映射到General-purpose input/output
* check_free_list: 检查空闲物理页表
* irq_init： 初始化中断控制，初始化内存中LOCAL_BASE、KERNBASE相关地址的值

本地初始化：
* lvbar: 设置vbar_el1，所有内核都有的寄存器
* timer_init：设置了cntp_ctl_el0、cntp_tval_el0、core timer（[14.1.3 Timers](https://cs140e.sergio.bz/docs/ARMv8-A-Programmer-Guide.pdf)）

所以通过加锁，使得有些初始化函数是只能被调用一次：
```C++
static struct spinlock conslock;
static int cpu = -1;
acquire(&conslock);
    if (cpu < 0){
        cpu = cpuid();
        ... global init
    }
release(&conslock)
... local init
```