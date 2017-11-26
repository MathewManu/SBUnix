#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

int main(int argc, char *argv[], char *envp[]);
void exit(int status);

void *malloc(size_t size);
void free(void *ptr);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int  munmap(void *addr, size_t length);


char *getenv(const char *name);
int setenv(char *name, char *value, int overwrite);
long syscall(int syscall_number, ...);
int  waitpid(int pid, int *st_ptr, int options);
#define __NR_read       0
#define __NR_write      1
#define __NR_open       2
#define __NR_close      3
#define __NR_mmap       9
#define __NR_munmap     11
#define __NR_pipe       22
#define __NR_dup2       33
#define __NR_fork       57
#define __NR_execve     59  
#define __NR_exit       60
#define __NR_wait4      61
#define __NR_getdents   78
#define __NR_getcwd     79
#define __NR_chdir      80
#define __NR_getdents64 217
#define __NR_waitid     247
#endif
