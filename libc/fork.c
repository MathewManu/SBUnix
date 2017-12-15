#include <stdlib.h>
#include <unistd.h>

pid_t fork() {
  return syscall(__NR_fork);
}
