#ifndef _STDIO_H
#define _STDIO_H

static const int EOF = -1;
typedef unsigned long	size_t;

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);

char *gets(char *s);

#endif
