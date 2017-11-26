#include <sys/terminal.h>
#include <sys/utils.h>
#include <sys/kprintf.h>
#include <sys/kb.h>

terminal_t terminal;

void init_terminal() {
  memset(&terminal, 0, sizeof(terminal));
  terminal.buffer[0] = '>';
  terminal.buffer[1] = ' ';
  terminal.buffer[2] = '_';
  terminal.buffer_offset = 3;
  terminal.buffer_ready = TERMINAL_BUFFER_NOT_READY;

  terminal_display(terminal.buffer);
}

static void reset_terminal() {
  init_terminal();
}

void clear_terminal() {
  char *temp1 = (char *)VIDEO_VIRT_MEM_BEGIN + 160*18;

  int terminal_size = 480;
  while (terminal_size > 0) {
	  *temp1 =' ';
    temp1 += CHAR_WIDTH;
    terminal_size--;
  }
}

void terminal_display(const char *fmt) {
  int row = 18;
  int col = 4;
  char *c;
  char *temp = (char *)VIDEO_VIRT_MEM_BEGIN + 160*18;

  clear_terminal();

  for (c = (char *)fmt; *c; c += 1, temp += CHAR_WIDTH) {

    if (row > 23) {
      kprintf("23...\n");
      memcpy((char *)VIDEO_VIRT_MEM_BEGIN + 160*18, (char *)VIDEO_VIRT_MEM_BEGIN + 160*18 + SCREEN_WIDTH, 800);
      temp -= SCREEN_WIDTH;
      clear_chars(temp, SCREEN_WIDTH);
      row = 23;
    } 
    /* Line wrapping */
    if (col == 81) {
      row++;
      col = 1;
      c -= 1;
      temp -= CHAR_WIDTH;
      continue;

    } else {
      *temp = *c;
      col++;
    }
  }
}

static void process_terminal_buffer() {

  /* TODO : Send the buffer content to the foreground process. This is just a demo implementation */
  terminal.buffer[terminal.buffer_offset - 1] = '\n';
  terminal.buffer[terminal.buffer_offset] = '\0';
  write_to_terminal(&terminal.buffer[2], strlen(&terminal.buffer[2]));
}

void handle_keyboard_input(unsigned char glyph, int flags) {

  if (flags == KEYCODE_BACKSPACE) {
    /* Backspace */
    if (terminal.buffer_offset > 3) {
      terminal.buffer[terminal.buffer_offset - 2] = '_';
      terminal.buffer[terminal.buffer_offset - 1] = ' ';
      terminal.buffer_offset--;
      terminal_display(terminal.buffer);
    }

  } else if (flags == KEYCODE_CTRL) {
    /* Control */
    if (glyph == 'M') {
      /* Enter key pressed */
      terminal.buffer[terminal.buffer_offset - 1] = '\0';
      terminal.buffer_ready = TERMINAL_BUFFER_READY;

      process_terminal_buffer();

      reset_terminal();
    }

  } else {
    /* Normal characters */
    if (terminal.buffer_offset < TERMINAL_BUFFER_SIZE - 1) {
      terminal.buffer[terminal.buffer_offset - 1] = glyph;
      terminal.buffer[terminal.buffer_offset] = '_';
      terminal.buffer_offset++;
      terminal_display(terminal.buffer);
    }
  }
}
void Sleep_t() {
    volatile int spin = 0;
      while (spin < 90000000) {
            spin++;
              }
}

void write_to_terminal(const char *buff, int size) {

  int i = 0;
  while (size > 0) {
    kprintf("%c", buff[i++]);
    size--;
  }
  Sleep_t();

}
