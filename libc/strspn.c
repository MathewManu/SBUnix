#include <stdio.h>
#include <string.h>

size_t strspn(char *s1, char *s2)
{
    size_t ret=0;
    while(*s1 && strchr(s2,*s1++))
        ret++;
    return ret;
}

