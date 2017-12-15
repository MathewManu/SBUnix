#include <stdlib.h>
#include <unistd.h>

int munmap(void *addr, size_t length) {
  return syscall(__NR_munmap, addr, length);
}

