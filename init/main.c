#include "printk.h"
#include "sbi.h"
#include "defs.h"
#include "proc.h"

extern void test();

int start_kernel() {
    printk("2021");
    printk("[S-MODE]  Hello RISC-V\n");
    schedule(); //add in lab5
    test(); // DO NOT DELETE !!!

	return 0;
}
