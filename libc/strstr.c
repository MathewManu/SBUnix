#include <stdio.h>
#include <string.h>

char *strstr(char *str1, char *str2) {
  char *s1 = str1, *s2 = str2, *ret = NULL;
  int match_len = 0;

  if (!str1 || !str2 || !*str1 || !*str2)
    return ret;

  while (*s1 && *s2) {
    if (strlen(s1) < strlen(s2))
      break;

    if (*s1 == *s2) {
      match_len++;
      if (!ret)
        ret = s1;
      s2++;
    } else {
      match_len = 0;
      s2 = str2;
      ret = NULL;
    }
    s1++;
  }

  if (match_len != strlen(str2))
    ret = NULL;

  return ret;
}

