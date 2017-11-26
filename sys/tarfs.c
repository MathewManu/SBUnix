#include <sys/tarfs.h>
#include <sys/defs.h>
#include <sys/utils.h>
#include <sys/kprintf.h>

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

    /* Skipping the entries with empty names. TODO: find out why they are present (Piazza 429) */
    if (strlen(hdr->name))
      kprintf("tarfs content : name = %s, size = %d\n", hdr->name, file_size);

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }
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

    if (!strncmp(hdr->name, bin_filename, strlen(bin_filename))) {
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

