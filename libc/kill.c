#include <stdlib.h>
#include <unistd.h>

int kill(int pid) {
  return syscall(__NR_kill, pid);
}

