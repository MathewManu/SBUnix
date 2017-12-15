#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

char *gets(char *s) {
  char buff[4096] = {0};
  syscall(__NR_read, 0, (void *)buff, sizeof(buff));

  if (strlen(buff)) {
    strcpy(s, buff);
    return s;
  }

  return NULL;
}
