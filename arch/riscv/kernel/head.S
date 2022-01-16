.extern start_kernel
.extern task_init
.extern mm_init
.extern early_pgtbl

    .section .text.init 
    .globl _start
_start:
    # ------------------
    # - your code here -
    # ------------------
    # set stvec = _traps

    la sp, boot_stack_top

    call setup_vm
    call relocate
    call mm_init
    call setup_vm_final

    call task_init    
    la t2, _traps
    csrw  stvec,  t2
    # set sie[STIE] = 1
    li t2,32
    csrs sie, t2

    # set first time interrupt
    rdtime t1
    li t2, 10000000
    add t2,t1,t2
    mv a7, x0
    mv a6,x0
    mv a0, t2
	mv a1, x0
	mv a2, x0
	mv a3,x0
	mv a4, x0
	mv a5, x0
    ecall

    # # set sstatus[SIE] = 1
    # li t2,0x2
    # csrs sstatus, t2 ; not used in lab5


    call start_kernel

relocate:

    li t1, 0xffffffe000000000; # VM_START
    li t2, 0x0000000080000000 ; # PHY_START
    sub t1, t1, t2 ; # t1 =  PA2VA_OFFSET = VM_START - PHY_START

    add ra, ra, t1 ; # set ra = ra + PA2VA_OFFSET
    add sp, sp, t1 ; # set sp = sp + PA2VA_OFFSET

    # set satp with early_pgtbl
    la t3,  early_pgtbl;
    srli t3, t3, 12; # PPN = PA >>  12
    li t1,  0x8000000000000000;
    or t3, t1, t3;
    csrw satp, t3;
    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 0x1000 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top:
