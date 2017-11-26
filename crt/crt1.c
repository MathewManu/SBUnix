#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
  __asm__(
    "xorl %ebp, %ebp;"

    "popq %rdi;"
    "movq %rsp, %rsi;"
    "movq %rsp, %rdx;"


    "pushq %rax;"
    "pushq %rsp;"
     //call main
    "call main;"

     //exit here.
    "movl $1, %eax;"
    "xorl %ebx, %ebx;"
    "int $0x80"
  );
}

