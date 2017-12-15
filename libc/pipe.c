#include <stdlib.h>
#include <unistd.h>

int pipe(int pipefd[2]) {
  return syscall(__NR_pipe, pipefd);
}

