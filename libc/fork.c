#include<stdlib.h>

pid_t fork() {
  return syscall(__NR_fork);
}
