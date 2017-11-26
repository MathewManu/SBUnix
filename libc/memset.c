#include <stdio.h>
#include <string.h>

void *memset(void *dest, int ch, size_t num_bytes) {
  char *tmp = dest;
  while (num_bytes) {
    *tmp++ = ch;
    num_bytes--;
  }
  return dest;
}

