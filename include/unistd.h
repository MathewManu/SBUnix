#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

//constants.
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define MAP_PRIVATE     0x02
#define MAP_ANONYMOUS   0x20
#define MAP_FAILED      ((void *)-1)

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002

#define S_IREAD         0000400
#define S_IWRITE        0000200

#define __NR_read       0
#define __NR_write      1
#define __NR_open       2
#define __NR_close      3
#define __NR_mmap       9
#define __NR_munmap     11
#define __NR_brk        12
#define __NR_pipe       22
#define __NR_dup2       33
#define __NR_fork       57
#define __NR_execve     59      
#define __NR_exit       60
#define __NR_wait4      61
#define __NR_kill       62 
#define __NR_getdents   78
#define __NR_getcwd     79
#define __NR_chdir      80
#define __NR_validexe   88
#define __NR_free       89
#define __NR_ps         90
#define __NR_opendir    93
#define __NR_readdir    94
#define __NR_closedir   95
#define __NR_ls         96
#define __NR_sleep      97
#define __NR_getdents64 217
#define __NR_waitid     247

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();

int execvpe(const char *file, char *const argv[], char *const envp[]);

pid_t wait(int *status);
int  waitpid(int pid, int *status);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
//int mkdir(const char *pathname, mode_t mode);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);


//functions add
int dup2(int oldfd, int newfd);
int ps();
int validexe(char *);
int cd(char *pathname);

#endif
