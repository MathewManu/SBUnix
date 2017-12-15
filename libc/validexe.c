#include <stdlib.h>
#include <unistd.h>

int validexe(char *filename) {
  return syscall(__NR_validexe, filename);
}

