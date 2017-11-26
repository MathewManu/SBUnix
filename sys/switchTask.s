.section .text
.global switch_task
switch_task:

#TODO: remove these. use incase 
# we need to save inside structure.
#    mov %r15, 8(%rdi)
 #   mov %r14, 16(%rdi)
  #  mov %r13, 24(%rdi)
   # mov %r12, 32(%rdi)
   # mov %rbx, 40(%rdi)
   # mov %rbp, 48(%rdi)

    push %r15
    push %r14
    push %r13
    push %r12
    push %rbx
    push %rbp

    push %rax
    push %rcx
    push %rdx

    #swap rsp
    mov %rsp, (%rdi)
    mov (%rsi), %rsp

    pop %rdx
    pop %rcx
    pop %rax
    pop %rbp
    pop %rbx
    pop %r12
    pop %r13
    pop %r14
    pop %r15

    #mov 8(%rsi), %r15
    #mov 16(%rsi), %r14
    #mov 24(%rsi), %r13
    #mov 32(%rsi), %r12
    #mov 40(%rsi), %rbx
    #mov 48(%rsi), %rbp

    ret #This ends all!
