#include <sys/syscall.h>
#include <sys/task.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/gdt.h>
#include <sys/utils.h>
#include <sys/terminal.h>
#include <sys/dirent.h>
#include <sys/tarfs.h>
#include <sys/timer.h>
#include <stdarg.h>

#define MSR_LSTAR   0xc0000082 
#define MSR_STAR    0xc0000081

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002

#define __NR_syscall_max     100 
#define __NR_read            0
#define __NR_write           1
#define __NR_open            2
#define __NR_close           3
#define __NR_brk             12
#define __NR_exit            60 
#define __NR_fork            57 
#define __NR_execve          59 
#define __NR_wait4           61
#define __NR_kill            62 
#define __NR_getcwd          79
#define __NR_chdir           80
#define __NR_validexe        88
#define __NR_free            89
#define __NR_ps              90
#define __NR_getpid          91
#define __NR_getppid         92
#define __NR_opendir         93
#define __NR_readdir         94
#define __NR_closedir        95
#define __NR_ls              96
#define __NR_sleep           97 


typedef void (*sys_call_ptr_t) (void);
sys_call_ptr_t sys_call_table[__NR_syscall_max];

static syscall_args_t syscall_args;

void copy_syscall_args_to_task_struct(task_struct_t *task) {

  (task->syscall_args).rdi = syscall_args.rdi;
  (task->syscall_args).rsi = syscall_args.rsi;
  (task->syscall_args).rdx = syscall_args.rdx;
  (task->syscall_args).r10 = syscall_args.r10;
  (task->syscall_args).r8  = syscall_args.r8;
  (task->syscall_args).r9  = syscall_args.r9;
  (task->syscall_args).rcx = syscall_args.rcx;
  (task->syscall_args).__NR_syscall = syscall_args.__NR_syscall;

}

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

  __asm__ __volatile__("pushq %rax;");
  __asm__ __volatile__("popq %%rax;movq %%rax, %0;":"=a"(syscall_args.__NR_syscall));
  
  get_syscall_args();

  copy_syscall_args_to_task_struct(get_current_running_task());

  /* not sure if doing right..above two operations would happen in userstack */
  __asm__ __volatile__("movq %%rsp, %0"  : "=a"(get_current_running_task()->ursp));
  __asm__ __volatile__("movq %0, %%rsp" :: "a"(get_current_running_task()->rsp));
  
  /* register bound variable to save the return value 
   * normal variables would be allocated in the stack. And we
   * are switching stack down the line. so "ret" value won't be
   * available to return.. so we should use register variables */
  //register int64_t ret __asm__("r15") = 0;
  /*ret =*/ (*sys_call_table[syscall_args.__NR_syscall])();
//  if(syscall_args.__NR_syscall != __NR_execve)
	 yield();

  
  __asm__ __volatile__("movq %%rsp, %0" : "=a"(get_current_running_task()->rsp));

 /* stack change from rsp to ursp */ 
  __asm__ __volatile__("movq %0, %%rsp" :: "a"(get_current_running_task()->ursp));
  
  /* restoring rcx register value, rip <-- rcx upon sysretq */
  __asm__ volatile("mov %0, %%rcx" ::"a"((get_current_running_task()->syscall_args).rcx));
  __asm__ volatile("add $0x8, %rsp"); 

  /* return value of syscall*/
  //__asm__ volatile("movq %0, %%rax"::"a"(ret));
  __asm__ volatile("movq %0, %%rax"::"a"(get_current_running_task()->retV));

  __asm__ volatile("sysretq"); 

}

uint32_t get_high_dword(uint64_t qword) {
  return (uint32_t)(qword >> 32);
}

uint32_t get_low_dword(uint64_t qword) {
  return (uint32_t)qword;
}

int get_fd(task_struct_t *task) {
  int fd_index = 3;
  while (fd_index < MAX_NUM_FDS) {
    if (task->fd_list[fd_index].fd == 0) {
      task->fd_list[fd_index].fd = 1;
      return fd_index;
    }

    fd_index++;
  }

  return INVALID_FD;
}

void free_fd(task_struct_t *task, int fd_index) {
  task->fd_list[fd_index].fd = 0;
  task->fd_list[fd_index].flags = 0;
  task->fd_list[fd_index].file_node = NULL;
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

  void *ptr;
  uint64_t size;
  char buff[512];
  ptr = (void*)syscall_args.rsi;
  size = syscall_args.rdx;

  memcpy(buff, ptr, size);
  write_to_terminal(buff, size);  
  
  get_current_running_task()->retV = 1;
}

void sys_read() {

  uint64_t fd;
  void *ptr;
  uint64_t size;
  int ret = -1;

  ptr = (void*)(syscall_args.rsi);
  fd = (int)(syscall_args.rdi);
  size = (uint64_t)(syscall_args.rdx);

  while (ret == -1) {
    if (fd == STDIN) {
      ret = read_from_terminal(ptr, size);

    } else if (fd > STDERR) {

      if (get_current_running_task()->fd_list[fd].flags == O_WRONLY) {
        ret = -1;
        break;
      } else {

        if (get_current_running_task()->fd_list[fd].file_node->file_type == FILE_TYPE_DIR) {
          ret = -1;
          break;

        } else {
          
          int tarfs_read_size = (get_current_running_task()->fd_list[fd].file_node->file_end - 
                                 get_current_running_task()->fd_list[fd].file_node->file_cursor);
          if ((tarfs_read_size > 0) && (tarfs_read_size > size)) {
            tarfs_read_size = size;
            memcpy(ptr, (void *)((get_current_running_task()->fd_list[fd].file_node)->file_cursor), tarfs_read_size);
            get_current_running_task()->fd_list[fd].file_node->file_cursor += tarfs_read_size;
            ret = tarfs_read_size;
          } else {
            ret = 0; 
          }
        }
      }
    }
  }

  get_current_running_task()->retV = ret;
}

void sys_exit() {

  get_current_running_task()->parent_task->task_state = TASK_STATE_RUNNING;
  set_task_state(TASK_STATE_STOPPED);
}

void sys_getcwd() {

  void *ptr;
  uint64_t size;

  ptr = (void *)(syscall_args.rdi);
  size = syscall_args.rsi;

  if (size >= strlen(get_current_running_task()->cwd)) {
    strncpy(ptr, "/", 1);
    strcpy(ptr + 1, get_current_running_task()->cwd);
    get_current_running_task()->retV = 1;

  } else {
    get_current_running_task()->retV = 0;
  }
}

void sys_ps() {

  task_struct_t *tmp = running_task;
  kprintf("\n------------------------------------");
  kprintf("\n PID  |  PPID  |  NAME  |  STATE  \n");
  kprintf("------------------------------------\n");
  while (tmp && tmp->next != running_task) {
    kprintf(" %d    |   %d  |   %s  |   %s\n", tmp->pid, tmp->ppid, tmp->name, task_state_str[tmp->task_state]);
    tmp = tmp->next;
  }

  if (tmp && tmp != running_task) {
    kprintf(" %d    |   %d  |   %s  |   %s\n", tmp->pid, tmp->ppid, tmp->name, task_state_str[tmp->task_state]);
  }
  kprintf("------------------------------------\n");
}

void sys_kill() {

  int pid = (int)((pid_t)(syscall_args.rdi));

  task_struct_t *cur = get_current_running_task();
  task_struct_t *tsk = cur;
  while (tsk->next != cur) {
    if(tsk->next->pid == pid) {
      tsk->next->task_state = TASK_STATE_STOPPED;
      //release_pid(tsk->next->pid);
      tsk->next = tsk->next->next;
      break;
    }
    tsk = tsk->next;
  }
}

void sys_sleep() {

  int sleep = syscall_args.rdi;
  int tot_time = get_time_since_up() + sleep;
  int curr;
  while (1) {
    curr = get_time_since_up();	
    if(curr >= tot_time)
	{
	break;
	}
  }
}

void sys_getpid() {

  get_current_running_task()->retV = get_current_running_task()->pid;
}

void sys_getppid() {

  get_current_running_task()->retV = get_current_running_task()->ppid;
}

void sys_wait() {
  get_current_running_task()->task_state = TASK_STATE_WAITING;
}

void sys_execve() {

  char *filename = (char *)syscall_args.rdi;

  set_cr3(get_cr3());

  char **argv = (char **)(syscall_args.rsi);

  execve_handler(filename, argv);

  (get_current_running_task()->syscall_args).rcx= get_current_running_task()->rip;
}

void sys_opendir() {

  char *name;
  name = (char *)(syscall_args.rdi);

  /* TODO : Name is not copied in dir for now.. */
  char pathname[128] = {0};
  file_t *node  = NULL;
  DIR *dir      = NULL;

  if (name[0] == '.' && name[1] == '.') {
    /* TODO : Handling pending */

  } else if (name[0] == '/') {

    node = find_node((char *)&name[1]);
    if (node) {
      dir = (DIR *)vmm_alloc_page();
      dir->node = node;
      if (node->num_children) {
        strcpy(dir->curr_dentry.d_name, node->child_node[0]->file_name);
      }
    }
  } else {

    int len = strlen(get_current_running_task()->cwd);
    if (name[0] == '.' && name[1] == '/') {

      strncpy(pathname, get_current_running_task()->cwd, len);
      strncpy(pathname + len, (char *)&name[1], strlen((char *)&name[1]));
      node = find_node(pathname);
      if (node) {
        dir = (DIR *)vmm_alloc_page();
        dir->node = node;
        if (node->num_children) {
          strcpy(dir->curr_dentry.d_name, node->child_node[0]->file_name);
        }
      }
    } else {

      strncpy(pathname, get_current_running_task()->cwd, len);
      strncpy(pathname + len, "/", 1);
      strncpy(pathname + len + 1, (char *)name, strlen((char *)name));
      node = find_node(pathname);
      if (node) {
        dir = (DIR *)vmm_alloc_page();
        dir->node = node;
        if (node->num_children) {
          strcpy(dir->curr_dentry.d_name, node->child_node[0]->file_name);
        }
      }
    }
  }

  get_current_running_task()->retV = (int64_t)dir;
}

void sys_readdir() {

  DIR *dirp = (DIR *)(syscall_args.rdi);

  struct dirent *d_ent = NULL;
  if (dirp && dirp->curr_child < dirp->node->num_children + 2) {

    if (dirp->curr_child == 0) {
      memset(dirp->curr_dentry.d_name, 0, sizeof(dirp->curr_dentry.d_name));
      strcpy(dirp->curr_dentry.d_name, ".");

    } else if (dirp->curr_child == 1) {
      memset(dirp->curr_dentry.d_name, 0, sizeof(dirp->curr_dentry.d_name));
      strcpy(dirp->curr_dentry.d_name, "..");

    } else {
      memset(dirp->curr_dentry.d_name, 0, sizeof(dirp->curr_dentry.d_name));
      if (dirp->node->child_node[dirp->curr_child - 2]->file_type == FILE_TYPE_DIR) {
        strcpy(dirp->curr_dentry.d_name, dirp->node->child_node[dirp->curr_child - 2]->file_name);
        strcpy(dirp->curr_dentry.d_name + strlen(dirp->curr_dentry.d_name), "/");
      } else {
        strcpy(dirp->curr_dentry.d_name, dirp->node->child_node[dirp->curr_child - 2]->file_name);
      }
    }

    d_ent = &(dirp->curr_dentry);
    dirp->curr_child++;
  }

  get_current_running_task()->retV = (int64_t)d_ent;
}

void sys_closedir() {

  DIR *dirp = (DIR *)(syscall_args.rdi);

  if (dirp) {
    vmm_dealloc_page((uint64_t)dirp);
    get_current_running_task()->retV = 0;

  } else {
    get_current_running_task()->retV = 1;
  }
}

void sys_open() {

  char      *name;
  uint16_t  flags;
  uint16_t  fd_index;
  name = (char *)(syscall_args.rdi);
  flags = (uint16_t)(syscall_args.rsi);

  char pathname[128] = {0};
  file_t *node  = NULL;

  get_current_running_task()->retV = -1;

  if (name[0] == '.' && name[1] == '.') {
    /* TODO : Handling pending */

  } else if (name[0] == '/') {

    node = find_node((char *)&name[1]);
    if (node) {
      fd_index = get_fd(get_current_running_task());
      get_current_running_task()->fd_list[fd_index].fd = fd_index;
      get_current_running_task()->fd_list[fd_index].flags = flags;
      get_current_running_task()->fd_list[fd_index].file_node = node;
      get_current_running_task()->retV = fd_index;
    }
  } else {

    int len = strlen(get_current_running_task()->cwd);
    if (name[0] == '.' && name[1] == '/') {

      strncpy(pathname, get_current_running_task()->cwd, len);
      strncpy(pathname + len, (char *)&name[1], strlen((char *)&name[1]));
      node = find_node(pathname);
      if (node) {
        fd_index = get_fd(get_current_running_task());
        get_current_running_task()->fd_list[fd_index].fd = fd_index;
        get_current_running_task()->fd_list[fd_index].flags = flags;
        get_current_running_task()->fd_list[fd_index].file_node = node;
        get_current_running_task()->retV = fd_index;
      }
    } else {

      strncpy(pathname, get_current_running_task()->cwd, len);
      strncpy(pathname + len, "/", 1);
      strncpy(pathname + len + 1, (char *)name, strlen((char *)name));
      node = find_node(pathname);
      if (node) {
        fd_index = get_fd(get_current_running_task());
        get_current_running_task()->fd_list[fd_index].fd = fd_index;
        get_current_running_task()->fd_list[fd_index].flags = flags;
        get_current_running_task()->fd_list[fd_index].file_node = node;
        get_current_running_task()->retV = fd_index;
      }
    }
  }
}

void sys_close() {

  int fd = (int)(syscall_args.rdi);

  get_current_running_task()->fd_list[fd].file_node->file_cursor = get_current_running_task()->fd_list[fd].file_node->file_begin;
  get_current_running_task()->fd_list[fd].file_node = 0;
  free_fd(get_current_running_task(), fd);
  get_current_running_task()->retV = 0;
}

void sys_cd() {

  char newpath[1024] = {0};     
  char *path = (char *)(syscall_args.rdi);

  if(path[strlen(path) - 1] == '/') {
    path[strlen(path) - 1] = '\0';
  }

  if (strlen(path)) {

    if (path[0] == '.' && path[1] == '.') {

      memset(newpath, 0, sizeof(newpath));
      char *saveptr;
      char *sep = "/";
      char arr[1024] = {0};
      strcpy(arr, get_current_running_task()->parent_task->cwd);

      char *token = strtok_r(arr, sep, &saveptr);
      while (token != NULL) {

        if (!strlen(newpath)) {
          strcpy(newpath, token);

        } else {

          if (strlen(newpath) + strlen(token) + 1 < strlen(get_current_running_task()->parent_task->cwd)) {
            strcpy(newpath + strlen(newpath), "/");
            strcpy(newpath + strlen(newpath), token);
          }
        }

        token = strtok_r(NULL, sep, &saveptr);
      }

      memset(get_current_running_task()->parent_task->cwd, 0, sizeof(get_current_running_task()->parent_task->cwd));
      strcpy(get_current_running_task()->parent_task->cwd, newpath);

    } else if (path[0] == '/') {
      memset(get_current_running_task()->parent_task->cwd, 0, sizeof(get_current_running_task()->parent_task->cwd));
      strcpy(get_current_running_task()->parent_task->cwd, &path[1]);

    } else {
      memset(newpath, 0, sizeof(newpath));
      strcpy(newpath, get_current_running_task()->parent_task->cwd);
      strcpy(newpath + strlen(newpath), "/");
      if (path[0] == '.' && path[1] == '/') {
        strcpy(newpath + strlen(newpath), &path[2]);
      } else {
        strcpy(newpath + strlen(newpath), path);
      }

      file_t *node = find_node(newpath);
      if (node) {
        memset(get_current_running_task()->parent_task->cwd, 0, sizeof(get_current_running_task()->parent_task->cwd));
        strcpy(get_current_running_task()->parent_task->cwd, newpath);

      } else {
        kprintf("cd: %s: No such file or directory\n", path);
      }
    }
  } else {
    memset(get_current_running_task()->parent_task->cwd, 0, sizeof(get_current_running_task()->parent_task->cwd));
    strcpy(get_current_running_task()->parent_task->cwd, "rootfs");
  }
}

void sys_brk() {

  uint64_t size = (uint64_t)(syscall_args.rdi);
  get_current_running_task()->retV = 0;

  if (size > 0) {
    vma_struct_t *vma = NULL;
    mm_struct_t *mm = get_current_running_task()->mm;
    if (mm) {
      vma = mm->mmap;
      while (vma) {
        if (vma->vma_type == VMA_TYPE_HEAP) {
          break;
        }
        vma = vma->vma_next;
      }

      if (vma && vma->vma_mm) {
        if (vma->vma_mm->brk + VIRT_PAGE_SIZE < vma->vma_end) {
          uint64_t ret = vma->vma_mm->brk;
          set_cr3(get_cr3());
          alloc_segment_mem(ret);
          vma->vma_mm->brk += VIRT_PAGE_SIZE;

          get_current_running_task()->retV = ret;
        }
      }
    }
  }
}

void sys_free() {

  uint64_t vaddr = (uint64_t)(syscall_args.rdi);
  get_current_running_task()->retV = 0;

  if (vaddr) {
    set_cr3(get_cr3());
    vmm_dealloc_page(vaddr);
  }
}

void sys_validexe() {

  char *filename = (char *)(syscall_args.rdi);
  get_current_running_task()->retV = 0;

  Elf64_Ehdr *elf_header = get_elf_header(filename);
  if (elf_header && elf_header->e_type == 2) {
    get_current_running_task()->retV = 1;
  }
}

/*
 * setting up syscall table init 
 */
void setup_sys_call_table() {

  sys_call_table[__NR_read]     = sys_read;  
  sys_call_table[__NR_write]    = sys_write;  
  sys_call_table[__NR_open]     = sys_open;  
  sys_call_table[__NR_close]    = sys_close;  
  sys_call_table[__NR_exit]     = sys_exit;  
  sys_call_table[__NR_fork]     = sys_fork;  
  sys_call_table[__NR_execve]   = sys_execve;  
  sys_call_table[__NR_wait4]    = sys_wait;  
  sys_call_table[__NR_getcwd]   = sys_getcwd;  
  sys_call_table[__NR_ps]       = sys_ps;  
  sys_call_table[__NR_kill]     = sys_kill;  
  sys_call_table[__NR_sleep]    = sys_sleep;  
  sys_call_table[__NR_getpid]   = sys_getpid;  
  sys_call_table[__NR_getppid]  = sys_getppid;  
  sys_call_table[__NR_opendir]  = sys_opendir;  
  sys_call_table[__NR_readdir]  = sys_readdir;  
  sys_call_table[__NR_closedir] = sys_closedir;  
  sys_call_table[__NR_chdir]    = sys_cd;
  sys_call_table[__NR_brk]      = sys_brk;
  sys_call_table[__NR_free]     = sys_free;
  sys_call_table[__NR_validexe] = sys_validexe;
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
