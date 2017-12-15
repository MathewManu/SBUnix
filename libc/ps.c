#include <stdlib.h>
#include <unistd.h>

int ps() {
  return syscall(__NR_ps);
}

