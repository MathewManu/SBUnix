#include <stdlib.h>
#include <unistd.h>
#include <sys/dirent.h>

struct dirent *readdir(DIR *dirp) {
  return (struct dirent *)syscall(__NR_readdir, dirp);
}

