#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

int main(int argc, char *argv[], char *envp[]);
void exit(int status);

void *malloc(size_t size);
void free(void *ptr);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int  munmap(void *addr, size_t length);


char *getenv(const char *name);
int setenv(char *name, char *value, int overwrite);
long syscall(int syscall_number, ...);

#endif
