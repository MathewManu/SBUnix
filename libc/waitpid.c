#include<stdlib.h>

int waitpid(int pid, int *st_ptr, int options) {
  return syscall(__NR_wait4, pid, st_ptr, options, NULL);
}

