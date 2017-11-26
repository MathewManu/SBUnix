#include <string.h>

int strcmp (const char *f_str, const char *s_str) {

  while (*f_str && (*f_str == *s_str)) {
    f_str++; s_str++;
  }
  return (*(unsigned char *)f_str - *(unsigned char *)s_str);
}

