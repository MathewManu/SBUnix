#include <sys/tarfs.h>
#include <sys/defs.h>
#include <sys/utils.h>
#include <sys/kprintf.h>
#include <sys/dirent.h>
#include <sys/vmm.h>

#define DUMP_TARFS_TREE {\
              kprintf("TARFS Tree : \n");\
              dump_tarfs_tree(tarfs_tree, 1);\
             }

/* tarfs file structure tree */
file_t *tarfs_tree;

file_t *get_tarfs_tree() {
  return tarfs_tree;
}

/*
 * Dumps the tarfs file structure tree
 * When invoked, the value for 'level' should be 1
 */
void dump_tarfs_tree(file_t *temp, int level) {

  if (temp) {
    int j = level;
    while(j--) {
      kprintf("   ");
    }

    kprintf("%s\n", temp->file_name);

    int i = 0;
    while (i < temp->num_children) {
      dump_tarfs_tree(temp->child_node[i], level + 1);
      i++;
    }
  }
}

uint64_t get_size_tar_octal(char *data, size_t size) {
  char *curr = (char *)data + size;
  char *ptr = curr;
  uint64_t sum = 0;
  uint64_t multiply = 1;

  while (ptr >= (char *) data) {
    if ((*ptr) == 0 || (*ptr) == ' ') {
      curr = ptr - 1;
    }
    ptr--;
  }

  while (curr >= (char *) data) {
    sum += ASCII_TO_NUM(*curr) * multiply;
    multiply *= 8;
    curr--;
  }

  return sum;
}

/* Given a binary filename, return the pointer to its ELF header */
Elf64_Ehdr *get_elf_header(char *bin_filename) {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    if (!strcmp(hdr->name, bin_filename)) {
      return (Elf64_Ehdr *)(hdr + 1);
    }

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }

  return NULL;
}

/* Go through tarfs and print the names of all directories and files
 */
void browse_tarfs() {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    if (strlen(hdr->name))
      kprintf("tarfs content : name = %s, size = %d\n", hdr->name, file_size);

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }
}

void update_tarfs_tree(char *file_name, uint64_t file_size, uint8_t file_type, uint64_t file_begin, uint64_t file_end) {

  if (!file_name)
    return;

  char *saveptr;
  char *sep = "/";
  char arr[256] = {0};
  strcpy(arr, file_name);

  file_t *temp = tarfs_tree;
  char *token = strtok_r(arr, sep, &saveptr);
  while (token != NULL) {
    int i = 0; int j = temp->num_children;
    while (i < temp->num_children) {
      if (!strcmp(temp->child_node[i]->file_name, token)) {
        temp = temp->child_node[i];
        break;
      }
      i++;
    }

    if (i == j) {
      temp->child_node[temp->num_children++] = create_child_node(temp, token, file_size, file_type, file_begin, file_end);
      temp = temp->child_node[temp->num_children - 1];
    }

    token = strtok_r(NULL, sep, &saveptr);
  } 
}

void init_tarfs_tree() {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  tarfs_tree = (file_t *)vmm_alloc_page();
  strcpy(tarfs_tree->file_name, "rootfs");
  tarfs_tree->file_type     = FILE_TYPE_DIR;
  tarfs_tree->file_size     = 0;
  tarfs_tree->parent_node   = tarfs_tree;
  tarfs_tree->num_children  = 0;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    if (strlen(hdr->name)) {
      if (hdr->typeflag[0] == FILE_TYPE_DIR) {
        update_tarfs_tree(hdr->name, file_size, hdr->typeflag[0], 0, 0);
      } else if (hdr->typeflag[0] == FILE_TYPE_FILE) {
        update_tarfs_tree(hdr->name, file_size, hdr->typeflag[0], (uint64_t)(hdr + 1), (uint64_t)((void *)hdr + sizeof(*hdr) + file_size));
      }
    }

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }
}

file_t *create_child_node(file_t *parent_node, char *file_name, uint64_t file_size, uint8_t file_type, uint64_t file_begin, uint64_t file_end) {

  file_t *child_node = (file_t *)vmm_alloc_page();
  strcpy(child_node->file_name, file_name);
  child_node->file_type     = file_type;
  child_node->file_size     = file_size;
  child_node->file_begin    = file_begin;
  child_node->file_end      = file_end;
  child_node->file_cursor   = file_begin;
  child_node->parent_node   = parent_node;
  child_node->num_children  = 0;

  return child_node;
}

file_t *find_node(char *name) {

  //kprintf("TARFS = [%s]\n", name);
  char *saveptr;
  char *sep = "/";
  char arr[256] = {0};
  strcpy(arr, name);

  uint8_t found = 0;

  if (!strcmp("rootfs", arr) || !strcmp("rootfs/", arr))
    return tarfs_tree;

  file_t *temp = tarfs_tree;
  char *token = strtok_r(arr, sep, &saveptr);
  if (token) {
    if (strcmp(token, "rootfs"))
      return NULL;
    token = strtok_r(NULL, sep, &saveptr);
  }

  while (token != NULL) {
    //kprintf("TOK = [%s]\n", token);
    int i = 0;
    found = 0;

    while (i < temp->num_children) {
      if (!strcmp(temp->child_node[i]->file_name, token)) {
        temp = temp->child_node[i];
        found = 1;
        break;
      }

      i++;
    }

    if (!found) {
      temp = NULL;
      break;
    }

    token = strtok_r(NULL, sep, &saveptr);
  }

  return temp;
}

