#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <utils.h>

int puts(char *s)
{
  return write(1, s, strlen(s));
}

int printf(const char *format, ...) {

  char buff[1024] = {0};
  char sbuff[1024] = {0};
	va_list args;
	va_start(args, format);

  char *s = buff;
  char *st = 0;
  int str_len;
  int int_arg;
  int q = 0;
  unsigned long gg;

  while (*format) {
    if (*format != '%') {
      *s = *format;
      s++;
      format++;
      continue;
    }
    format++; 
    switch (*format) {
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
        write(1, "Unknown format specifier\n", strlen("Unknown format specifier\n"));
    }

    format++;
  }
  
  va_end(args);

  write(1, buff, strlen(buff));

  return 0;
}

