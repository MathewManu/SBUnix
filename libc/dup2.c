#include<unistd.h>
#include<stdlib.h>

int dup2(int oldfd, int newfd) {
  return syscall(__NR_dup2, oldfd, newfd);
}

