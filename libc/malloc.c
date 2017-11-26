#include<stdlib.h>
#if 0
void *malloc(size_t sz) {
  int *mem_ptr;
  mem_ptr = (int *)mmap(0, sz + sizeof(sz), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  if (mem_ptr == MAP_FAILED)
    return NULL;

  *mem_ptr = sz + sizeof(sz);
  return (void *)(mem_ptr + 1);
}

#endif
