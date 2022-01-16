//arch/riscv/kernel/proc.c
#include "proc.h"

extern void __dummy();
extern unsigned long swapper_pg_dir[512];
extern char uapp_start[];
extern char uapp_end[];
struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此

void task_init() {
    /* YOUR CODE HERE */
    printk("proc_init begin, this jin haian 310106083!\n");
    
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    idle = kalloc(); // allocate a page for idle
    // 2. 设置 state 为 TASK_RUNNING;
    idle->state =  TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    idle-> counter = idle->priority = 0;
    // 4. 设置 idle 的 pid 为 0
    idle-> pid = 0;
    // 5. current, task[0] both point to idle
    current = idle;
    task[0] = idle;
    

    // lab 3
    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址


    // lab5  update
    // 1. 对每个用户态进程，其拥有两个 stack： U-Mode Stack 以及 S-Mode Stack，
    //      其中 S-Mode Stack 在 lab3 中我们已经设置好了。
    //      我们可以通过 kalloc 接口申请一个空的页面来作为 U-Mode Stack。
    // 2. 为每个用户态进程创建自己的页表 并将 uapp 所在页面，
    //      以及 U-Mode Stack 做相应的映射，同时为了避免 U-Mode 
    //      和 S-Mode 切换的时候切换页表，我们也将内核页表 （ swapper_pg_dir ） 
    //      复制到每个进程的页表中。
    //3. 对每个用户态进程我们需要将 sepc 修改为 USER_START， 设置 sstatus 中的 SPP 
    // （使得 sret 返回至 U-Mode ）， SPIE （ sret 之后开启中断 ）， 
    //    SUM （ S-Mode 可以访问 User 页面 ）， sscratch 设置为 U-Mode 的 sp，
    //    其值为 USER_END （即 U-Mode Stack 被放置在 user space 的最后一个页面）。
    /* YOUR CODE HERE */
    for(int i = 1; i < NR_TASKS; i++) {
            task[i] = (struct task_struct *)kalloc(); 
            // printk("now at: 0x%lx\n",(unsigned long)task[i] );
            task[i]->state =  TASK_RUNNING;
            task[i]->counter = 0;
            task[i]->priority = rand();
            task[i]->pid = i;
            task[i]->thread.ra = __dummy;
            task[i]->thread.sp = (uint64)task[i] + PGSIZE;

            unsigned long * user_stack = kalloc();
            // create a dedicated root page table for every user program
            unsigned long* rootPGT = kalloc();

            task[i]->pgd = (unsigned long)rootPGT - PA2VA_OFFSET;
             // every user page should have the content of kernal page 
            //  memcopy(rootPGT,  swapper_pg_dir, PGSIZE); 
            for (int i = 0; i < 512; i++)
            {
                rootPGT[i] =  swapper_pg_dir[i];
            }
            
            // map user stack and user program from VA to PA
            // We can notice that several user processes may have the VA mapping to different PA
            // stack 
            create_mapping(rootPGT,  USER_END-PGSIZE ,  (unsigned long )user_stack - PA2VA_OFFSET,  PGSIZE , 0b10111);
            // user program
            create_mapping(rootPGT,  USER_START ,  (unsigned long)uapp_start-PA2VA_OFFSET, (unsigned long)uapp_end -  (unsigned long)uapp_start, 0b11111);
            task[i]->thread.sstatus =  csr_read(sstatus);
            //set SUM(bit 18, so kernel mode can access user mode page), 
            //set SPIE(bit 5, so interruption is enabled after sret), 
            // set SPP to be 0, so after calling mret, the system can return to user mode 
            task[i]->thread.sstatus = task[i]->thread.sstatus | 0x00040020; //set SUM(bit 20, so kernel mode can access user mode page)
            csr_write(sstatus, task[i]->thread.sstatus); // ?
            task[i]->thread.sepc =  USER_START;
            task[i]->thread.sscratch = USER_END;

    }

    printk("...proc_init done!\n");
}


// arch/riscv/kernel/proc.c

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            // printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
            printk("[PID = %d] is running! thread space begin at %lx \n", current->pid, current);
        }
    }
}

// arch/riscv/kernel/proc.c

extern void __switch_to(struct task_struct* prev, struct task_struct* next);

void switch_to(struct task_struct* next) {
    if (current == next) {
        return ;
    }
    else {
        // printk("New process will be scheduled\n");
        struct task_struct*  current_temp = current;
            current = next;
        __switch_to(current_temp, next);
        return ;
    }
}


// arch/riscv/kernel/proc.c

void do_timer(void) {
    /* 1. 如果当前线程是 idle 线程 直接进行调度 */
    /* 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减 1 
          若剩余时间任然大于0 则直接返回 否则进行调度 */
    if (current == idle) {
        // printk("idle will be shedule\n");
        schedule();
    }
    else  if (--current->counter == 0 ) {
        schedule();
    }
    else return;
    
}


#ifdef SJF
void schedule(void) {
    int min_time = 0x7fffffff; //max
    int min_index = 0;

    while(1) {
        for (int i = 1; i < NR_TASKS; i++) {
            if ((task[i]->state == TASK_RUNNING) && (task[i]->counter > 0) && (task[i]->counter < min_time) ) {
                min_time = task[i]->counter;
                min_index = i;
            }
        }
        if (min_index > 0) {
            // printk("A min time has been found!\n");
            break;
        }
        printk("\n");
        for(int i = 1; i < NR_TASKS; i++) {
            task[i]->counter = rand();
            printk("SET [PID = %d COUNTER = %d]\n", i,  task[i]->counter);
        }
        printk("\n");
    }
    // printk("Context switch begin \n");
    
    // printk("switch to [PID = %d COUNTER = %d]\n", task[min_index]->pid, task[min_index]->counter);
    switch_to(task[min_index]) ;

}
#endif

#ifdef PRIORITY


//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    int min_time = 0x7fffffff; //max
    int next_index = 0;
    int max_priority = 0;

    while(1) {
        for (int i = 1; i < NR_TASKS; i++) {
            if ((task[i]->state == TASK_RUNNING) && (task[i]->counter > 0) && (task[i]->priority >= max_priority) ) {
                
                if ((task[i]->priority == max_priority && task[i]->counter < min_time) ||  task[i]->priority > max_priority ) {
                        min_time = task[i]->counter; 
                        next_index = i;
                    // when the priority is the same, chooose the process with least running time, otherwise choose the process with larger priority
                }

                max_priority = task[i]->priority ;
            }
        }
        if (next_index > 0) {
            break;
        }
        printk("\n");
        for(int i = 1; i < NR_TASKS; i++) {
            task[i]->counter = (task[i]->counter >> 1) + task[i]->priority ;
            printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", i,  task[i]->priority, task[i]->counter);
        }
        printk("\n");
    }
    // printk("Context switch begin \n");
    
    // printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", task[next_index]->pid, task[next_index]->priority, task[next_index]->counter);
    switch_to(task[next_index]) ;
}

#endif
