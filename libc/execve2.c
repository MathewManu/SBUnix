#include<stdlib.h>
#include<unistd.h>

int execve2(const char *filename, char *argv, char *const envp[]) {
  return syscall(__NR_execve, filename, argv, envp);
}

