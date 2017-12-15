#include <stdlib.h>
#include <unistd.h>
#include <sys/dirent.h>

int closedir(DIR *dirp) {
  return syscall(__NR_closedir, dirp);
}

