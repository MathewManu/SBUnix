#include <tcl/stdlib.h>
#include <sys/kprintf.h>

extern uint32_t *loader_stack;

void *Tcl_Malloc(int size) {

  static void *physfree = NULL;
  static int i = 0;

  if (i == 0) {
    physfree = (uint64_t *)(uint64_t)loader_stack[4];
  }

  void *tmp = physfree;  
  physfree += size;

  i++;
  return tmp;
}

