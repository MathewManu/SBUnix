.section .text
.global switchring3
switchring3:
     cli
     mov $0x23, %rbx
     mov %rbx, %ds
     mov %rbx, %es
     mov %rbx, %fs
     mov %rbx, %gs
                   
     mov %rsp, %rax
     
     # 1) Pushing DS index
     pushq $0x23

     # 2) Pushing RSP
     #pushq %rax

     #trying to push user stack addrress.
     pushq %rcx

     # 3) Pushing Flags
     pushf 

     popq %rax
     or $0x200, %rax
     pushq %rax

     # 4) Pushing CS index
     pushq $0x2B

     # 5) Pushing EIP (function pointer)
     pushq %rdi 

     iretq
