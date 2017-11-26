long syscall(int syscall_number, ...) {
  long ret;
  __asm__(
  "mov    %%rdi,%%rax;"
  "mov    %%rsi,%%rdi;"
  "mov    %%rdx,%%rsi;"
  "mov    %%rcx,%%rdx;"
  "mov    %%r8,%%r10;"
  "mov    %%r9,%%r8;"
  "mov    0x8(%%rsp),%%r9;"
  "syscall;"
  "cmp    $0xfffffffffffff001,%%rax;"
  :"=r"(ret)
  );

  return ret;
}

int write(int fd, const void *c, int size) {
  return syscall(1, fd, c, size);
}

int main() {

  char ch[7] = "manu123";

  write(1, &ch, 7);
  write(1, &ch, 7);
  write(1, &ch, 7);

  return 0;
}
