#include <stdlib.h>
#include <unistd.h>

int chdir(const char *path) {
  return syscall(__NR_chdir, path);
}
