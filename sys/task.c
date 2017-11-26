#include <sys/task.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/gdt.h>
#include <sys/utils.h>
#include <sys/elf64.h>

static task_struct_t *running_task;
static task_struct_t main_task;

static task_struct_t task1;
static task_struct_t task2;

/*
 * just for testing 
 * TODO:not required. just to verify switching print statement
 */
void Sleep() {
  volatile int spin = 0;
  while (spin < 50000000) {
    spin++;
  }
}

/*
 * TODO: syscall from our libc code. 
 * this should never be inside /sys code. Move later.
 */
#define __NR_write      1

long syscall(int syscall_number, ...) {
  long ret;
  __asm__ volatile(
  "mov    %%rdi,%%rax;"
  "mov    %%rsi,%%rdi;"
  "mov    %%rdx,%%rsi;"
  "mov    %%rcx,%%rdx;"
  "mov    %%r8,%%r10;"
  "mov    %%r9,%%r8;"
  "mov    0x8(%%rsp),%%r9;"
  "syscall;"
  "cmp    $0xfffffffffffff001,%%rax;"
  :"=r"(ret)
  );

  /* not sure if rax has be updated manually from ring0 */
  return ret;
}

uint64_t write(int fd, const void *c, size_t size) {
  return syscall(__NR_write, fd, c, size);
}

/* till this part, code from libc */

void ring3func() {

  kprintf("Inside user land function Calling write here.\n");

  char ch[20]="String from Ring 3\n";
  write(1, &ch, strlen(ch));

  kprintf("Returned to userland ring3 from ring0 after sysret\n");
  // __asm__ volatile("" ::"a"(syscall_no+1));
  //__asm__ volatile("syscall");
  kprintf("Ring 3 : while 1.\n");

  while(1);
}

void switch_to_user_mode() {
  uint64_t cs = get_user_cs() | 0x3;
  uint64_t ds = get_user_ds() | 0x3;

  //switchring3(ring3func, cs, ds);
  switchring3((void *)0x4000B0, cs, ds);
}

void task1Main() {
    while(1) {
        kprintf("Thread ####1\n");
        Sleep();
        set_tss_rsp((void *)task1.ctx.rsp);
				yield();
    }
}
 
void task2Main() {
    while(1) {
        kprintf("Thread ####2\n");
        Sleep();
        set_tss_rsp((void *)task1.ctx.rsp);
        switch_to_user_mode();
        kprintf("dwInside Dummy....\n");
				yield();
    }
}

/*
 * init tasking.. creating two tasks. task1 & task2
 * initializing the tasks as follows.
 * maintask ---> task1 <------> task2
 * task1 & task2 are in a cycle.
 * TODO: not pusing cr3 & EFLAGS now. may need later.
 */
void init_tasking() {
    // Get EFLAGS and CR3
    //__asm__ volatile("mov %%cr3, %%eax; mov %%eax, %0;":"=m"(main_task.ctx.cr3)::"%eax");
    // __asm__ volatile("mov %%cr3, %0": "=r"(main_task.ctx.cr3));
    //__asm__ volatile("pushfl; movq (%%esp), %%eax; movq %%eax, %0; popfl;":"=m"(main_task.ctx.eflags)::"%eax");
 
    //create_task(&task1, task1Main, main_task.ctx.eflags, (uint64_t*)main_task.ctx.cr3);
    create_task_nw(&task1, task1Main);
    create_task_nw(&task2, task2Main);

    main_task.next = &task1;
    task1.next = &task2;
    task2.next = &task1;
 
    running_task = &main_task;
}
 
void create_task_nw(task_struct_t *task, void (*main)()) {
    task->ctx.rsp = (uint64_t) &(task->kstack[4016]);;
    /* placing main's address, func pointer in the stack
     * towards the end. kstack is a char array, in order to 
     * save a 64 bit address in the stack, first creating a
     * uint64_t pointer to kstack. Then saving main's address.
     */
    uint64_t *tmp_ptr = (uint64_t *)&(task->kstack[4088]);
    *tmp_ptr = (uint64_t) main;
}
 
void yield() {
    task_struct_t *last = running_task;
    running_task = running_task->next;
    switch_task(last, running_task);
}

/*
 * call from main.c comes here in doIt
 * This can be moved to main.c itself
 * TODO: should it be in main???
 */
void doIt() {
    yield();
}

void execute_user_process(char *bin_filename) {
  /*
  context     ctx;
  char        kstack[4096];
  struct task_struct_t *next;
  uint64_t    pid;
  uint64_t    ppid;
  uint64_t    rip;
  uint64_t    cr3;
  char        name[32];
  mm_struct_t *mm;
  */
  task_struct_t *task = &task2;
  
  pml4_t *pml4 = (pml4_t *)pmm_alloc_block();
  pml4_t *new_pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pml4_t *kern_pml4 = (pml4_t *)((uint64_t)get_kernel_pml4() | VIRT_ADDR_BASE);
  new_pml4->pml4_entries[511] = kern_pml4->pml4_entries[511];

  set_cr3(pml4);

  task->mm = (mm_struct_t *)vmm_alloc_page();
//  while(1);
  alloc_segment_mem(0x400000);
//
  load_binary(task, bin_filename);
}

