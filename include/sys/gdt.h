#ifndef _GDT_H
#define _GDT_H

void init_gdt();
void set_tss_rsp(void *rsp);
uint64_t get_user_ds();
uint64_t get_user_cs();

#endif
