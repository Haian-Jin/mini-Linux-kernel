// arch/riscv/kernel/vm.c

extern unsigned long  _stext;
extern unsigned long  _srodata;
extern unsigned long  _sdata;
extern unsigned long  _sbsss;

# include "defs.h"
# include <string.h>
#include <stddef.h>
#include "mm.h"
#include "printk.h"
#include "types.h"


/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    early_pgtbl[2] = ((unsigned long)(0x2<< 28)) | 0x000000000000000f;
    early_pgtbl[384] = ((unsigned long)(0x2<< 28)) | 0x000000000000000f;
}


unsigned long  *get_the_PTE_addr(unsigned long *root, unsigned long va) {
    unsigned long * cur_ptes_page_addr = root;
    unsigned long *  cur_pte_addr;
    for(int level = 2; level > 0; level--) {  
        if (level == 2) {
            cur_pte_addr = &cur_ptes_page_addr[(va>>30) & (PAGE_ENTRY_NUM - 1)];
        }
        else if (level == 1) {
            cur_pte_addr = &cur_ptes_page_addr[(va>>21) & (PAGE_ENTRY_NUM - 1)];
        }
        if ((*cur_pte_addr)&0x1) //valid 
        {
            cur_ptes_page_addr = (unsigned long *)((((*cur_pte_addr) >> 10) << 12) + PA2VA_OFFSET);
        }
        else // allocate a free page, and *cur_pte_addr will point to the new free page
        {           
            if ((cur_ptes_page_addr = (uint64 *)kalloc()) == NULL) {
                printk("\n!!!No free mem space left!!!.\n");
                return NULL;  
            }
            
            memset(cur_ptes_page_addr, 0, PGSIZE);
            // set the valid bit of the *cur_pte_addr be 1 ana make it point to the next level 
            // printk("%lx \n",((unsigned long)cur_ptes_page_addr -  PA2VA_OFFSET) >> 12);
            WritePTE(cur_pte_addr, ((unsigned long)cur_ptes_page_addr -  PA2VA_OFFSET) >> 12, 0, 1);
        } 
    }
    return  &cur_ptes_page_addr[(va>>12) & (PAGE_ENTRY_NUM - 1)]; 
}


/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    unsigned long  va_now = va;
    unsigned long pa_now = pa;
    //according to va and pa( both are base address), set the mapping page by page. The offset is sz
    for (va_now = va; va_now < va + sz; pa_now+=PGSIZE, va_now+=PGSIZE) {
        unsigned long * PTE_addr =  get_the_PTE_addr(pgtbl, va_now);
        // printk("%lx\n", PTE_addr);
        WritePTE(PTE_addr, ((unsigned long)pa_now) >> 12, perm, 1);
        
    }
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);
    unsigned long * pgtbl = swapper_pg_dir;

    // No OpenSBI mapping required
    unsigned long va = VM_START + OPENSBI_SIZE, pa = PHY_START + OPENSBI_SIZE;
    // mapping kernel text X|-|R|V
    unsigned long text_length = U64ADDR(_srodata) - U64ADDR(_stext); 
    // printk("text:\n,va: %lx \n, pa: %lx \n, length:%lx\n ",  va, pa, text_length);
   create_mapping(pgtbl, va, pa, text_length  , 0b1011);
    va += text_length;
    pa +=  text_length;

    // mapping kernel rodata -|-|R|V
    unsigned long rodata_length = U64ADDR(_sdata) - U64ADDR(_srodata);
    create_mapping(pgtbl, va, pa, rodata_length  , 0b0011);

    va += rodata_length;
    pa += rodata_length;

    // mapping other memory -|W|R|V
    unsigned long left_length = PHY_SIZE - rodata_length - text_length -OPENSBI_SIZE;   //128 MB - rodata - text
    //printk("other memory: %lx ~ %lx\n", va, va+left_length);
    create_mapping(pgtbl, va, pa, left_length   , 0b0111);

    // set satp with swapper_pg_dir
    unsigned long temp = ((unsigned long)pgtbl) - PA2VA_OFFSET;
    temp = ((unsigned long)temp) >> 12;
    temp = (0x000fffffffffff & temp) | 0x8000000000000000; 

    csr_write(satp, temp);
    // flush TLB
    asm volatile("sfence.vma zero, zero");

    return;
}
