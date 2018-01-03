#include <sys/task.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/gdt.h>
#include <sys/utils.h>
#include <sys/elf64.h>
#include <sys/dirent.h>
#include <sys/tarfs.h>

#define CMD_LEN   1024
task_struct_t *running_task;
static task_struct_t main_task;

static task_struct_t task1;
static task_struct_t task2;

static uint8_t pid[MAX_NUM_PROCESSES] = {0};

char task_state_str[][32] = {"UNKNOWN",
                             "READY",
                             "RUNNING",
                             "WAITING",
                             "STOPPED"
                            };
/* Allocate an available process id */
uint32_t allocate_pid() {
  static uint16_t pid_index = 3;
  uint16_t ret_pid = INVALID_PID;

  while (pid_index < MAX_NUM_PROCESSES) {
    if (pid[pid_index] == 0) {
      pid[pid_index] = 1;
      ret_pid = pid_index;
      pid_index++;

      if ((pid_index % MAX_NUM_PROCESSES) == 0)
        pid_index = 3;

      break;
    }

    pid_index++;
    if ((pid_index % MAX_NUM_PROCESSES) == 0)
      pid_index = 3;
  }

  return ret_pid;
}

void release_pid(uint16_t pid_index) {
  if (pid_index < MAX_NUM_PROCESSES) {
    pid[pid_index] = 0;
  }
}

uint8_t get_task_state() {
  return running_task->task_state;
}

void set_task_state(uint8_t state) {
  running_task->task_state = state;
}

void free_task_memory(task_struct_t *task) {

  release_pid(task->pid);

  vmm_dealloc_page((uint64_t)(task->kstack));

  pml4_t *curr_cr3 = get_cr3();
  set_cr3((pml4_t *)(task->cr3));

	mm_struct_t *mm = task->mm;
  vma_struct_t *vma = task->mm->mmap;
  while (vma != NULL) {
    vma_struct_t *del = vma;
    vma = vma->vma_next;
    vmm_dealloc_page((uint64_t)del);
  }

  vmm_dealloc_page((uint64_t)mm);

  set_cr3(curr_cr3);

  cleanup_page_table((pml4_t *)(task->cr3));
}

void cleanup_tasks() {

  task_struct_t *curr = get_current_running_task();
  task_struct_t *tmp = curr;
  while (tmp->next != curr) {
    /* TODO : take reference and clean up tmp->next before next statement */
    if(tmp->next->task_state == TASK_STATE_STOPPED) {

      task_struct_t *del = tmp->next;
      pml4_t *curr_cr3 = get_cr3();
      pml4_t *parent_cr3 = (pml4_t *)(del->parent_task->cr3);
      free_task_memory(tmp->next);

    	tmp->next = tmp->next->next;

      set_cr3(parent_cr3);
      vmm_dealloc_page((uint64_t)del); 
      set_cr3(curr_cr3);
    }
    tmp = tmp->next;
  }
  //kprintf("rsp [%p]\n", get_current_running_task()->rsp);
  //kprintf("get_num_free_blocks() %d\n", get_num_free_blocks());
  //kprintf("get_num_used_blocks() %d\n", get_num_used_blocks());
}

/* Returns the current running task reference */
task_struct_t *get_current_running_task() {
  return running_task;
}

/*
 * just for testing 
 * TODO:not required. just to verify switching print statement
 */
void Sleep() {
  volatile int spin = 0;
  while (spin < 90000000) {
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
  /* TODO : Replace 0x4000F0 with correct value read from sbush elf */
 // switchring3((void *)0x4000F0, cs, ds, 0x900000);
  switchring3((void *)0x4000F0, cs, ds, 0xFAA00000+4016);
}

void idle_func() {
	//static int c = 0;
	while(1) {
		//kprintf("Idle Func #### %d\n", c);
		//c++;
		//Sleep();
		set_tss_rsp((void *)task1.rsp);
		// switch_to_user_mode();
		//if(c%20 == 0) {
		  cleanup_tasks(); 
     // c = 0;
    //}
		yield();
	}
}
 
void init_sbush_proc() {
    switch_to_user_mode();
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
    //__asm__ volatile("mov %%cr3, %%rax; mov %%rax, %0;":"=m"(main_task.ctx.cr3)::"%rax");
    // __asm__ volatile("mov %%cr3, %0": "=r"(main_task.ctx.cr3));
    //__asm__ volatile("pushfl; movq (%%esp), %%eax; movq %%eax, %0; popfl;":"=m"(main_task.ctx.eflags)::"%eax");
 
    create_task(&task1, idle_func);
    create_task(&task2, init_sbush_proc);

    main_task.next = &task1;
    task1.next = &task2;
    task2.next = &task1;
 
    running_task = &main_task;
}
 
void create_task(task_struct_t *task, void (*main)()) {
    task->kstack = vmm_alloc_page();
    task->rsp = (uint64_t) (task->kstack + 4016);
    //task->ursp = 0x900000;
    task->parent_task = NULL;
    strcpy(task->cwd, "rootfs");

    memset(task->fd_list, 0, sizeof(task->fd_list));
    /* placing main's address, func pointer in the stack
     * towards the end. kstack is a char array, in order to 
     * save a 64 bit address in the stack, first creating a
     * uint64_t pointer to kstack. Then saving main's address.
     */
    uint64_t *tmp_ptr = (uint64_t *)(task->kstack + 4088);
    *tmp_ptr = (uint64_t) main;

     __asm__ volatile("mov %%cr3, %0": "=r"(task->cr3));
}

void switch_task(task_struct_t *old, task_struct_t *new) {

	__asm__ volatile(
		"pushq %r15;"
    "pushq %r13;"
    "pushq %r12;"
    "pushq %rbx;"
    "pushq %rbp;"

    "pushq %rcx;"
    "pushq %rdx;"
    "pushq %rax;"
    "pushq %r14;"

    "mov %rsp, (%rdi);"
    "mov (%rsi), %rsp;"

    "popq %r14;"
    "popq %rax;"
    "popq %rdx;"
    "popq %rcx;"

    "popq %rbp;"
    "popq %rbx;"
    "popq %r12;"
    "popq %r13;"
    "popq %r15;"

    );

  __asm__ __volatile__ (
          "movq %0, %%cr3;"
              ::"r"(new->cr3)
                );

  __asm__ __volatile__ ("ret;");

}

void yield() {
    task_struct_t *last = running_task;

    task_struct_t *next_task = running_task->next;

    while (next_task->task_state == TASK_STATE_STOPPED || next_task->task_state == TASK_STATE_WAITING) {
	    next_task = next_task->next;
    }

    if (running_task != next_task) {
      running_task = next_task;
      switch_task(last, running_task);
    }
}

/*
 * call from main.c comes here in doIt
 * This can be moved to main.c itself
 * TODO: should it be in main???
 */
void doIt() {
    yield();
}

static inline void invlpg(void* m)
{
      /* http://wiki.osdev.org/Inline_Assembly/Examples#INVLPG */
      __asm__ __volatile__ ( "invlpg (%0)" : : "b"(m) : "memory" );
}

void start_init_process() {
  task_struct_t *task = &task1;
  
  pml4_t *pml4 = (pml4_t *)pmm_alloc_block();
  pml4_t *new_pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pml4_t *kern_pml4 = (pml4_t *)((uint64_t)get_kernel_pml4() | VIRT_ADDR_BASE);
  new_pml4->pml4_entries[511] = kern_pml4->pml4_entries[511];

  set_cr3(pml4);
  task->mm = (mm_struct_t *)vmm_alloc_page();
  task->cr3 = (uint64_t) pml4;
  
  task->task_state = TASK_STATE_RUNNING;
  task->pid  = 1;
  task->ppid = 0;
  task->num_children = 0;
  strcpy(task->name, "init");
}

void start_sbush_process(char *bin_filename) {
  task_struct_t *task = &task2;
  
  pml4_t *pml4 = (pml4_t *)pmm_alloc_block();
  pml4_t *new_pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pml4_t *kern_pml4 = (pml4_t *)((uint64_t)get_kernel_pml4() | VIRT_ADDR_BASE);
  new_pml4->pml4_entries[511] = kern_pml4->pml4_entries[511];

  set_cr3(pml4);
  task->mm = (mm_struct_t *)vmm_alloc_page();
  task->cr3 = (uint64_t) pml4;
  
  task->task_state = TASK_STATE_RUNNING;
  task->pid  = 2;
  task->ppid = 0;
  task->num_children = 0;
  strcpy(task->name, "sbush");
  load_binary(task, bin_filename);
}

void set_c_task(task_struct_t *c_task, task_struct_t *p_task) {

  c_task->rsp  = p_task->rsp;
  c_task->rip  = p_task->rip;
  c_task->ursp = p_task->ursp;
  c_task->pid  = allocate_pid();
  p_task->retV = c_task->pid;
  c_task->ppid = p_task->pid;
  c_task->mm   = NULL;
  c_task->next = NULL;
  c_task->cr3  = (uint64_t)pmm_alloc_block();
  c_task->num_children = 0;
  c_task->task_state = TASK_STATE_RUNNING;
  c_task->kstack = vmm_alloc_page();
  c_task->rsp = (uint64_t)(c_task->kstack + 4016);
  c_task->parent_task = p_task;
  strcpy(c_task->name, p_task->name);
  strcpy(c_task->cwd, p_task->cwd);

  memcpy(c_task->fd_list, p_task->fd_list, sizeof(p_task->fd_list));
  memcpy(&(c_task->syscall_args), &(p_task->syscall_args), sizeof(p_task->syscall_args));
}

task_struct_t *copy_parent_task(task_struct_t *p_task) {

  task_struct_t *c_task = (task_struct_t *)vmm_alloc_page();

  set_c_task(c_task, p_task);
	create_child_paging(c_task->cr3);

	set_cr3((pml4_t *)c_task->cr3);

	c_task->mm = (mm_struct_t *)vmm_alloc_page();
	memcpy(c_task->mm, p_task->mm, sizeof(mm_struct_t));
	c_task->mm->mmap = NULL;

	vma_struct_t *p_vma = p_task->mm->mmap;
	vma_struct_t *c_vma = NULL;
	vma_struct_t *c_prev_vma = NULL;

	while (p_vma) {
		c_vma = (vma_struct_t *)vmm_alloc_page();
		memcpy(c_vma, p_vma, sizeof(vma_struct_t));
    c_vma->vma_next = NULL;
    c_vma->vma_mm = c_task->mm;

    if (c_prev_vma) {
      c_prev_vma->vma_next = c_vma;
    } else {
			c_task->mm->mmap = c_vma;
    }

		c_prev_vma = c_vma;
		p_vma = p_vma->vma_next;
	}

  (p_task->num_children)++;
	return c_task;
}

void sys_fork() {

  task_struct_t *parent_task = running_task;
  parent_task->cr3 = (uint64_t)get_cr3();

  task_struct_t *child_task = copy_parent_task(parent_task); 

  task_struct_t *temp = parent_task->next;
  parent_task->next = child_task;
  child_task->next  = temp;

  set_cr3((pml4_t *)parent_task->cr3);

  volatile uint64_t current_stack_loc;
  uint64_t parent_stack_top = (uint64_t)(parent_task->kstack + 4095);
  uint64_t child_stack_top = (uint64_t)(child_task->kstack + 4095);

  __asm__ __volatile__("movq %%rsp, %0"  : "=a"(current_stack_loc));

  memcpy((void *)(child_stack_top - (parent_stack_top - current_stack_loc)), (void *)current_stack_loc, parent_stack_top - current_stack_loc + 1);

  child_task->rsp = (child_stack_top - (parent_stack_top - current_stack_loc) - 72 + 40);

  child_task->retV = 0x0;
}

void execve_handler(char *file_name, char *argv[]) {

//  kprintf("EXECVE_HAND filename = [%s]\n", argv[0]);
//  kprintf("EXECVE_HAND filename = [%s]\n", argv[1]);
  char filename[CMD_LEN] = {0};
  strcpy(filename, file_name);

  /* arg passing code */
  int i = 0;
  int j = 0;
  int no_of_args = 0;

  char args[6][64] = {{0}};
  strcpy(args[i++], filename);
  no_of_args++;
  while (argv[j] && (i < 6)) {
//    kprintf("EXECVE_HAND = [%s]\n", argv[j]);
    strcpy(args[i], argv[j]);
    i++;
    j++;
    no_of_args++;
  }
  //kprintf("EXECVE_HAND filename = [%s]\n", filename);

  task_struct_t *cur_task = get_current_running_task();
  strcpy(cur_task->name, filename);

  Elf64_Ehdr *elf_header = get_elf_header(cur_task->name);
  if (elf_header == NULL) {
    kprintf("%s : command not found\n", cur_task->name);
    get_current_running_task()->task_state = TASK_STATE_STOPPED;
    if(get_current_running_task()->parent_task->task_state == TASK_STATE_WAITING)
	get_current_running_task()->parent_task->task_state = TASK_STATE_RUNNING;
    return;
  }

 // kprintf("EXECVE_HAND filename = [%s]\n", argv[0]);
  if (load_binary(cur_task, cur_task->name)) {
    get_current_running_task()->task_state = TASK_STATE_STOPPED;
  //  kprintf("failure");
//    if(get_current_running_task()->parent_task->task_state == TASK_STATE_WAITING)
//	get_current_running_task()->parent_task->task_state = TASK_STATE_RUNNING;
  }
  //kprintf("EXECVE_HAND filename = [%s]\n", argv[0]);

  //uint64_t *ursp_ptr = (uint64_t *)cur_task->ursp;
  void *ursp_ptr = (uint64_t *)cur_task->ursp;
  ursp_ptr -= (no_of_args*64);
  memcpy(ursp_ptr, (void*)args, no_of_args*64);
  //kprintf("---- %s \n", args[0]);
  i = no_of_args;
  /* push all the argvs to the user stack */
  while(i>0) {
    *(uint64_t*)(ursp_ptr-(i*8)) = ((uint64_t)ursp_ptr + ((no_of_args-i)*64));
    //kprintf("testing %p %p %s\n", (ursp_ptr-(i*8)), (ursp_ptr+(no_of_args-i)*64), *(uint64_t*)(ursp_ptr-(i*8)));
    i--;
  }
  ursp_ptr -= (no_of_args*8);
  /* push argc to the stack*/
  *(uint64_t*)(ursp_ptr-8) = no_of_args;
  ursp_ptr -= 8;
  cur_task->ursp = (uint64_t)ursp_ptr;
  cur_task->ursp -= 8 ;

}


