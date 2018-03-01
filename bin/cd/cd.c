#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {

  if (argc > 1) {
    cd(argv[1]);
  }

  exit(0);
}

