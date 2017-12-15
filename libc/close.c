#include <stdlib.h>
#include <unistd.h>

int close(int fd) {
  return syscall(__NR_close, fd);
}

