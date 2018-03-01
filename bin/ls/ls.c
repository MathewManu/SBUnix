#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/dirent.h>

int main(int argc, char *argv[], char *envp[]) {

  struct dirent *pDirent;
  DIR *pDir;

  char buff[128] = {0};
  getcwd(buff, 127);

  pDir = opendir(buff);
  if (pDir == NULL) {
    exit(1);
  }

  while ((pDirent = readdir(pDir)) != NULL) {
    write(1, pDirent->d_name, strlen(pDirent->d_name));
    write(1, "\n", 1);
  }

  write(1, "\n", 1);
  closedir(pDir);

  exit(0);
}

