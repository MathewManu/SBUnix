#include <string.h>

char *strncpy(char *dest, char *src, int len) {
  int i;
  for (i = 0; i < len && src[i] != '\0'; i++)  {
    dest[i] = src[i];
  }
  return dest;
}

