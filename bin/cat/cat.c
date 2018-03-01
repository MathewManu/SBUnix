#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_LEN  1024

void handle_cat(int num, char *files[]) {
  int i = 0;
  int leftover = 0;

  while (i < num) {
    int file = open(files[i], O_RDONLY);
    char code[BUFFER_LEN] = {0};
    size_t n = 0;
    char c;

    if (file == -1) {
      puts("cat: ");
      puts(files[i]);
      puts(": No such file or directory\n");
      return;
    }

    while (read(file, &c, 1) > 0)
    {
      leftover = 1;
      if (n < (BUFFER_LEN - 1)) {
        code[n++] = (char) c;

        if (c == '\n') {
          code[n - 1] = c;
          code[n] = '\0';
          puts(code);
          n = 0;
          leftover = 0;
        }
      }
    }

    if (leftover && (n < (BUFFER_LEN - 1))) {
      code[n] = '\n';
      code[n + 1] = '\0';
      puts(code);
    }

    close(file);
    i++;
  }
}

int main(int argc, char *argv[], char *envp[]) {

  if (argc > 1)
    handle_cat(argc - 1, &argv[1]);

  exit(0);
}

