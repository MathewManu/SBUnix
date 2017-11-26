#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>

extern char *strcpy(char *dest, char *src);
extern char *strncpy(char *dest, char *src, int num_bytes);
extern char *strtok_r(char *str, char *delim, char **nextp);
extern char *strstr(char *str1, char *str2);
extern int  strlen(char *str);
extern int  strncmp(const char *f_str, const char *s_str, size_t n);
extern int  strcmp(const char *f_str, const char *s_str);
extern void *memset(void *dest, int ch, size_t num_bytes);
extern size_t strspn(char *s1, char *s2);
extern size_t strcspn(char *s1, char *s2);
extern char *strchr(char *s, int c);

#endif /* _STRING_H */
