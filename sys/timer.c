#include <sys/timer.h>
#include <sys/kprintf.h>
#include <sys/utils.h>
#include <sys/task.h>

int time_since_up=10;
static int count;
static int sec;

int get_time_since_up() {
  return time_since_up;
}

void display_time(int secs) {
  int buff_len = 0;
  char sbuff[256] = {0};
  char *c;
  char *temp = (char *)VIDEO_VIRT_MEM_BEGIN + SCREEN_WIDTH * (SCREEN_HEIGHT - 1);
  clear_chars(temp, SCREEN_WIDTH - 10);

  buff_len += strlen(UP_TIME_PRINT);
       strncpy(sbuff, UP_TIME_PRINT, buff_len);

  int2char(secs, &sbuff[buff_len]);

  for (c = sbuff; *c; c += 1, temp += CHAR_WIDTH) {
      *temp = *c;
  }
}

void print_timer() {
  count += 1;

  if (count == 18) {
    sec += 1;
    time_since_up = sec;
    display_time(sec);
	//yield();
    count = 0;
  }
}
