#include <sys/kprintf.h>
#include <sys/utils.h>
#include <stdarg.h>

void display(const char *fmt) {
  static int row = 1;
  static int col = 1;
  char *c;
  static char *temp = (char *)VIDEO_VIRT_MEM_BEGIN;

  for (c = (char *)fmt; *c; c += 1, temp += CHAR_WIDTH) {
    if (row > 17) {
      memcpy((char *)VIDEO_VIRT_MEM_BEGIN, (char *)VIDEO_VIRT_MEM_BEGIN + SCREEN_WIDTH, 2660);
      temp -= SCREEN_WIDTH;
      clear_chars(temp, SCREEN_WIDTH);
      row = 17;
    }

    if (*c == '\n') {
      temp += 2 * (80 - col);
      col = 1;
      row++;
      continue;
    }

    if (col != 81) {
      clear_chars(temp, SCREEN_WIDTH - 2 * (col - 1));
      *temp = *c;
      col++;
    } else {
      display("\n");
      col = 1;
      c -= 1;
      temp -= CHAR_WIDTH;
      continue;
    }
  }
}


void kprintf(const char *fmt, ...)
{
  char buff[1024] = {0};
  char sbuff[1024] = {0};
	va_list args;
	va_start(args, fmt);

  char *s = buff;
  char *st = 0;
  int str_len;
  int int_arg;
  int q = 0;
  unsigned long gg;

  while (*fmt) {
    if (*fmt != '%') {
      *s = *fmt;
      s++;
      fmt++;
      continue;
    }
    fmt++; 
    switch (*fmt) {
      case 'c':
        *s++ = (unsigned char)va_arg(args, int);
        break;
      case 'd':
        q = 0;
        memset(sbuff, 0, sizeof(sbuff));
        int_arg = va_arg(args, int);
        int2char(int_arg, sbuff); 
        str_len = strlen(sbuff);
        while (str_len--) {
          *s++ = sbuff[q++];
        }
        break;
      case 's':
        st = va_arg(args, char *);
        str_len = strlen(st);
        while (str_len--) {
          *s++ = *st++;
        }
        break;
      case 'x':
        q = 0;
        memset(sbuff, 0, sizeof(sbuff));
        int_arg = va_arg(args, int);
  			convert(sbuff, int_arg, 16, 0);
        reverse(sbuff);
        str_len = strlen(sbuff);
        while (str_len--) {
          *s++ = sbuff[q++];
        }
        break;
      case 'p':
        q = 0;
        memset(sbuff, 0, sizeof(sbuff));
        sbuff[0] = '0';
        sbuff[1] = 'x';
        gg = (unsigned long)va_arg(args, unsigned long);
        convert(&sbuff[2], gg, 16, 0);
        reverse(&sbuff[2]);
        str_len = strlen(sbuff);
        while (str_len--) {
          *s++ = sbuff[q++];
        }
        break;
      default:
        display("Unknown format specifier\n");
    }

    fmt++;
  }
  
  va_end(args);

  display(buff);
}

