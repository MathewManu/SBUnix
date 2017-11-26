#ifndef _KB_H
#define _kB_H

#define KEYCODE_NORMAL    0
#define KEYCODE_CTRL      1
#define KEYCODE_BACKSPACE 2

void key_handler();
void display_glyph(unsigned char glyph, int flags);

#endif
