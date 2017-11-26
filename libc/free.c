#include<stdlib.h>

void free(void *mem_ptr) {
  if (mem_ptr) {
    munmap((void *)mem_ptr, *((int *)mem_ptr - 1));
  }
}

