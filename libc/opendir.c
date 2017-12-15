#include <stdlib.h>
#include <unistd.h>
#include <sys/dirent.h>

DIR *opendir(const char *name) {
  return (DIR *)syscall(__NR_opendir, name);
}

