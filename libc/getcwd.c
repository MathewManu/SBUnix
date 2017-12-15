#include <stdlib.h>
#include <unistd.h>

char *getcwd(char *buf, size_t size) {
  return (char *)syscall(__NR_getcwd, buf, size);
}

