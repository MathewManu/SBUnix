#include <stdlib.h>
#include <unistd.h>

void *malloc(size_t sz) {
  return (void *)(syscall(__NR_brk, sz));
}

