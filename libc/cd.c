#include <stdlib.h>
#include <unistd.h>

int cd(char *pathname) {
  return syscall(__NR_chdir, pathname);
}
