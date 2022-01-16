#include "syscall.h"
#include "stdio.h"


static inline long getpid() {
    long ret;
    asm volatile ("li a7, %1\n"
                  "ecall\n"
                  "mv %0, a0\n"
                : "+r" (ret) 
                : "i" (SYS_GETPID));
    return ret;
}

int main() {
    register unsigned long current_sp __asm__("sp");
    // printf("!!!!%d\n",  getpid());
    while (1) {
        printf("[U-MODE] pid: %ld, sp is %lx\n", getpid(), current_sp);
        // int a = getpid();
        // printf("[U-MODE] pid: %ld, sp is %lx\n", a, current_sp);
        // printf("[U-MODE] pid: %ld, sp is %lx\n", 1, current_sp);
        for (unsigned int i = 0; i < 0x4FFFFFFF; i++);
    }

    return 0;
}
