#include<stdlib.h>
#if 0
int chdir(const char *path) {
  return syscall(__NR_chdir, path);
}
#endif
