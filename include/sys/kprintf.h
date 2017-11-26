#ifndef __KPRINTF_H
#define __KPRINTF_H

#define VIDEO_PHYS_MEM_BEGIN  0xB8000
#define VIDEO_VIRT_MEM_BEGIN  0xFFFFFFFF800B8000
#define SCREEN_WIDTH          160
#define SCREEN_HEIGHT         25
#define ASCII_WIDTH           1
#define ATTR_WIDTH            1
#define CHAR_WIDTH            (ASCII_WIDTH + ATTR_WIDTH)

void kprintf(const char *fmt, ...);

#endif
