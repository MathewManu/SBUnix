#include <stdio.h>
#include <string.h>

size_t strcspn(char *s1, char *s2)
{
    size_t ret=0;
    while(*s1)
        if(strchr(s2,*s1))
            return ret;
        else
            s1++,ret++;
    return ret;
}

