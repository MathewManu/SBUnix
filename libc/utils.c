#include <sys/utils.h>
#include <sys/defs.h>

void clear_chars(char *temp, int size) {
  int i = 0;
  for (i = 0; i < size; i += 2) {
     temp[i] = 0;
  }
}

/* number base converter, int to oct, hex etc */
void convert(char *a, unsigned long n, int base, int i) {
  if (n == 0 && i == 0) {
    a[i] = 48;
    return;
  }

  int rem = n % base;
  if (n == 0)
    return;

  convert(a, n / base, base, i + 1);
  if (rem < 10) {
    a[i] = rem + 48;

  } else {
    a[i] = rem - 10 + 'A';
  }
}

void reverse(char *a) {
  int i;
  int j = strlen(a);
  char c;

  for (i = 0,j = strlen(a) - 1; i < j; i++, j--) {
    c = a[i];
    a[i] = a[j];
    a[j] = c;
  }
}

void int2char(int num, char *input) {
  int i = 0, r;
  if (num == 0) {
    input[i++] = 48;

  } else {

    while (num) {
      r = num % 10;
      num = num / 10;
      input[i++] = r + 48;
    }
  }

  reverse(input);
}

