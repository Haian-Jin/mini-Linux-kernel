#ifndef _DEFS_H
#define _DEFS_H

// #include "types.h"
// typedef unsigned long uint64;

#define csr_read(csr)                       \
({                                          \
    register unsigned long __v;                    \
    asm volatile ("csrr  %0,  " #csr " "         \
                    :  "=r" (__v)  \
                    : : "memory");                     \
    __v;                                    \
})

#define csr_write(csr, val)                         \
({                                                  \
    unsigned long __v = (unsigned long)(val);                     \
    asm volatile ("csrw " #csr ", %0"               \
                    : : "r" (__v)                   \
                    : "memory");                    \
})

#endif


#define PHY_START 0x0000000080000000
#define PHY_SIZE  128 * 1024 * 1024 // 128MB， QEMU 默认内存大小
#define PHY_END   (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000 // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE-1) & (~(PGSIZE - 1)))
// #define PGROUNDUP(addr) ((addr + PGSIZE) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))

#define USER_START (0x0000000000000000) // user space start virtual address
#define USER_END   (0x0000004000000000) // user space end virtual address

#define OPENSBI_SIZE (0x200000)

#define VM_START (0xffffffe000000000)
#define VM_END   (0xffffffff00000000)
#define VM_SIZE  (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

#define PAGE_ENTRY_NUM 0x200  // 512 bytes
#define WritePTE(__pte_addr,__ppn_to_write, __perm, __V)                                                                   \
{                                                                                                                                                                                               \
        *__pte_addr =   ((unsigned long)(*(__pte_addr)) & 0xffc0000000000000) |                                                    \
                                                ((unsigned long)(__ppn_to_write) << 10) | ((unsigned long)(__perm) | (unsigned long)(__V));             \
}

#define U64ADDR(num) ((unsigned long)&num)