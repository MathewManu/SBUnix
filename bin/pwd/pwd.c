#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
  char buff[128] = {0};
  getcwd(buff, 127);

  write(1, buff, strlen(buff));
  write(1, "\n\n", 2);

	exit(0);
}

