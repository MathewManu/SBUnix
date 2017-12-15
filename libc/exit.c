#include <stdlib.h>
#include <unistd.h>

void exit(int status) {
    syscall(__NR_exit, status);
}
