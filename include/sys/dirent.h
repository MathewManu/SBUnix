#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>

#define NAME_MAX 255

#define FILE_TYPE_DIR   0x35
#define FILE_TYPE_FILE  0x30

#define MAX_CHILDREN    256

#define STDIN		  0
#define STDOUT		1
#define STDERR		2

typedef struct dirent {
 uint64_t d_in;
 char     d_name[NAME_MAX+1];
} dirent;

typedef struct file_t {
  char      file_name[32];
  uint8_t   file_type;
  uint64_t  file_size;
  uint64_t  file_begin;
  uint64_t  file_end;
  uint64_t  file_cursor;
  uint16_t  num_children;
  struct file_t *parent_node;
  struct file_t *child_node[MAX_CHILDREN];
} file_t;

typedef struct DIR { 
  //int      fd;
  file_t   *node;
  uint16_t curr_child;
  dirent   curr_dentry;
  char     name[128];
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

file_t *create_child_node(file_t *parent_node, char *file_name, uint64_t file_size, uint8_t file_type, uint64_t file_begin, uint64_t file_end);

#endif
