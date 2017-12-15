#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <sys/defs.h> 

#define TERMINAL_BUFFER_SIZE      1024
#define TERMINAL_BUFFER_NOT_READY 0
#define TERMINAL_BUFFER_READY     1

typedef struct terminal_t {
  char      buffer[TERMINAL_BUFFER_SIZE];
  uint64_t  buffer_offset;
  uint64_t  buffer_ready;
} terminal_t;

extern terminal_t terminal;

void init_terminal();
void terminal_display(const char *fmt);
int read_from_terminal();
void write_to_terminal(const char *buff, int size);
//int write_to_terminal(const char *buff);
int read_from_terminal(char *buffer, int size);
void handle_keyboard_input(unsigned char glyph, int flags);

#endif
