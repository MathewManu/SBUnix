#include <tcl/string.h>

int my_strncmp (const char *f_str, const char *s_str, int n) {
  for ( ; n > 0; f_str++, s_str++, n--) {
    if (*f_str != *s_str) {
      return (*(unsigned char *)f_str < *(unsigned char *)s_str) ? -1 : 1;
    }
    else if(*f_str == '\0') {
      return 0;
    }
  }
  return 0;
}

