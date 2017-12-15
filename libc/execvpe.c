#include<stdlib.h>
#include<unistd.h>

int execvpe(const char *filename, char *const argv[], char *const envp[]) {
  return syscall(__NR_execve, filename, argv, envp);
}

