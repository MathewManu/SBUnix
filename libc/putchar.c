#include <unistd.h>

int putchar(int c) {
  char ch = c;
  if (write(1, &ch, 1) == 1)
    return c;

  return 0;
}
