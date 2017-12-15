#ifndef _UTILS_H
#define _UTILS_H

#include <sys/defs.h>

int strlen(char *str);
char *strcpy(char *dest, char *src);
char *strncpy(char *dest, char *src, int len);
void *memset(void *dest, int ch, int num_bytes);
void clear_chars(char *temp, int size);
void memcpy(void *dest, const void *src, int n);
void convert(char *a, unsigned long n, int base, int i);
void reverse(char *a);
void int2char(int num, char *input);
int strcmp(const char *f_str, const char *s_str);
int strncmp(const char *f_str, const char *s_str, int n);
char *strchr(char *s, int c);
size_t strcspn(char *s1, char *s2);
size_t strspn(char *s1, char *s2);
char *strtok_r(char *str, char *delim, char **nextp);

#endif

