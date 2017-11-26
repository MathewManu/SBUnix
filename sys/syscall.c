#include <sys/task.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/gdt.h>
#include <sys/utils.h>
#include <sys/terminal.h>

 
#define MSR_LSTAR   0xc0000082 
#define MSR_STAR    0xc0000081

#define __NR_syscall_max     50 
#define __NR_read            0
#define __NR_write           1

typedef void (*sys_call_ptr_t) (void);
sys_call_ptr_t sys_call_table[__NR_syscall_max];

typedef struct sycall_args_t {

  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t r10;
  uint64_t r8;
  uint64_t r9;

  uint64_t rcx;
  uint64_t __NR_syscall;

}syscall_args_t;

static syscall_args_t syscall_args;

void get_syscall_args() {

  __asm__ __volatile__(
      "pushq %r9;"
      "pushq %r8;"
      "pushq %r10;"
      "pushq %rdx;"
      "pushq %rsi;"
      "pushq %rdi;"
      "pushq %rcx;"
      );

  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.rcx));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.rdi));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.rsi));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.rdx));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.r10));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.r8));
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.r9));

}

/*
 * TODO: not saving rsp now.
 * save if required later.
 * saving rcx register. When sysretq is invoked, rip would be 
 * loaded with rcx value.
 */
void syscall_handler() {

  __asm__ __volatile__(
      "pushq %rax;");
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.__NR_syscall));
  
  get_syscall_args();

  (*sys_call_table[syscall_args.__NR_syscall])();
  
  /* restoring rcx register value, rip <-- rcx upon sysretq */
  __asm__ volatile("mov %0, %%rcx" ::"a"(syscall_args.rcx));
  __asm__ volatile("add $0x8, %rsp"); 
  __asm__ volatile("sysretq"); 

}

uint32_t get_high_dword(uint64_t qword) {
  return (uint32_t)(qword >> 32);
}

uint32_t get_low_dword(uint64_t qword) {
  return (uint32_t)qword;
}

/*
 * has to be saved seperately high & low part.
 * references: 
 * https://github.com/torvalds/linux/blob/ead751507de86d90fa250431e9990a8b881f713c/arch/x86/include/asm/msr.h
 */
static inline void __wrmsr(unsigned int msr, uint32_t low, uint32_t high) {
 __asm__ __volatile__("wrmsr" :: "a"(low), "d"(high), "c"(msr));

}

// setting the 0th(SCE) bit of IA32_EFER to enable syscall instruction.
static inline void enable_syscall_instr() {

	__asm__ __volatile__("xor %rcx, %rcx; \
											 mov $0xC0000080, %rcx; \
												rdmsr; \
											 or $0x1, %rax; \
											wrmsr");
}

/*TODO: move syscall functions somewhere else
 * as of now adding here itself
 *
 */
void sys_write() {

  uint64_t fd;
  void *ptr;
  uint64_t size;
  char buff[512];

  ptr = (void*)syscall_args.rsi;
  fd = syscall_args.rdi;
  size = syscall_args.rdx;

  memcpy(buff, ptr, size);
  write_to_terminal(buff, size);  
  
  kprintf("%d %d Inside sys_write handler\n", fd, size);
  kprintf("\n");
}

void sys_read() {
  uint64_t fd;
  void *ptr;
  uint64_t size;
  char buff[512];


  ptr = (void*)syscall_args.rsi;
  fd = syscall_args.rdi;
  size = syscall_args.rdx;

  memcpy(buff, ptr, size);
  kprintf("hellon %d %s %d\n", fd, buff, size);
  kprintf("\nsys_read dummy funtion. Ring 0\n");
}

/*
 * setting up syscall table init 
 */
void setup_sys_call_table() {

  sys_call_table[__NR_read] = sys_read;  
  sys_call_table[__NR_write] = sys_write;  
  /* add remaining syscalls here..*/

}

/*
 * setting lstar with the syscall_handler address.
 * setting star with required values.
 *
 * http://wiki.osdev.org/Sysenter#AMD:_SYSCALL.2FSYSRET
 * http://www.felixcloutier.com/x86/SYSCALL.html
 * https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-174.html
 *
 */
void init_syscall() {

  uint64_t star_reg_value = ((uint64_t)0x1b << 48) | ((uint64_t)0x8 << 32);

  __wrmsr(MSR_LSTAR, get_low_dword((uint64_t)syscall_handler), get_high_dword((uint64_t)syscall_handler)); 

  __wrmsr(MSR_STAR, get_low_dword(star_reg_value), get_high_dword(star_reg_value)); 

	enable_syscall_instr();
  
  setup_sys_call_table();

}
