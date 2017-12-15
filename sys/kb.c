#include <sys/kb.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/utils.h>
#include <sys/terminal.h>
/*
 * scancode reference: http://www.osdever.net/bkerndev/Docs/keyboard.htm
 *
 */

#define FLAG_CTRL     0x80
#define FLAG_SHIFT    0x40
#define FLAG_ALT      0x20

unsigned char scancode_arr[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
 '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

unsigned char scancode_arr_upper[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
 '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port));
    return ret;
}

void key_handler() {
  static unsigned char flag = 0;
  unsigned char scode;
  scode = inb(0x60);
  if (!(scode & 0x80)) {

    if (scode == 0x2A || scode == 0x36) {

      flag = FLAG_SHIFT;
      return;

    } else if (scode == 0x1D || scode == -0x1D) {

      flag = FLAG_CTRL;
      return;

    } else if (scode == 0x38 || scode == -0x38) {

      flag = FLAG_ALT;
      return;
    }
     
    if (flag & FLAG_CTRL) {
      display_glyph(scancode_arr_upper[scode], KEYCODE_CTRL);
    } else if (flag & FLAG_SHIFT) {
      display_glyph(scancode_arr_upper[scode], KEYCODE_NORMAL);
    } else {
      if (scode == 0x1C) {
        /* Enter pressed */
        display_glyph('M', KEYCODE_CTRL);
      } else if (scode == 0x0E) {
        /* Backspace pressed */
        display_glyph(' ', KEYCODE_BACKSPACE);
      } else {
        display_glyph(scancode_arr[scode], KEYCODE_NORMAL);
      }
    }

    flag = 0;
  }
}

void display_glyph(unsigned char glyph, int flags) {
#if 0
  unsigned char sbuff[6] = {0};
  unsigned char *c;
  int i = 0;
  char *temp = (char *)VIDEO_VIRT_MEM_BEGIN + SCREEN_WIDTH * SCREEN_HEIGHT - 10;
  clear_chars(temp, 10);

  sbuff[0] = '[';

  if (flags == KEYCODE_CTRL)
    sbuff[1] = '^';
  else
    sbuff[1] = ' ';

  sbuff[2] = glyph;
  sbuff[3] = ' ';
  sbuff[4] = ']';

  for (c = sbuff; i < 5; c += 1, i++, temp += CHAR_WIDTH) {
      *temp = *c;
  }
#endif

  handle_keyboard_input(glyph, flags);
}

