#include<stdlib.h>

int pipe(int pipefd[2]) {
  return syscall(__NR_pipe, pipefd);
}

