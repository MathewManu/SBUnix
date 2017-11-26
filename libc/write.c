#include<stdlib.h>
#include<unistd.h>

ssize_t write(int fd, const void *c, size_t size) {
  return syscall(__NR_write, fd, c, size);
}

