// trap.c 
#include "clock.h"
#include "printk.h"
#include "proc.h"
#include "syscall.h"

struct pt_regs {
    unsigned long x[32];
    unsigned long sepc;
    unsigned long sstatus;
};

void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs ) {
    /*
    Judge the type of trap based on scause. 
    If it is the time interruption, call clock_set_next_event() to set the next time interrupt 
    and decide if the current process needs to be scheduled or not in do_timer() function.
    Else if it is syscall, then based on system call nember stored in a7, the correponding system call 
    operation will be executed.
    This project can only support 2 types of syscall temporarily.
    */
    if (scause == 0x8000000000000005) {
        printk("[S] Supervisor Mode Timer Interrupt\n");
        clock_set_next_event();
        do_timer();
        
    }  else {
        unsigned long a0 = regs->x[10];
        unsigned long a1 = regs->x[11];
        unsigned long a2 = regs->x[12];
        unsigned long a7 = regs->x[17];
        if (scause == 8) {
            //printk("sys_number = %lx \n", a7);
            if (a7 == SYS_WRITE) {
                sys_write(a0, a1, a2);
            } else if (a7 == SYS_GETPID) {
                
                regs->x[10] = sys_getpid();

            }
            regs->sepc =(unsigned long)(((unsigned long)regs->sepc) + (unsigned long)0x4);
        }
    }
    return ;
}