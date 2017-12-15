#include <stdlib.h>
#include <unistd.h>

unsigned int sleep(unsigned int seconds) {
  return syscall(__NR_sleep, seconds);
}

