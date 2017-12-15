#include <stdlib.h>
#include <unistd.h>

void free(void *mem_ptr) {
  syscall(__NR_free, mem_ptr);
}

