#include <sys/elf64.h>
#include <sys/task.h>
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/utils.h>

int load_binary(task_struct_t *task, char *bin_filename) {

  Elf64_Ehdr *elf_header = get_elf_header(bin_filename);
  kprintf("elf header.. %p\n", elf_header);
  kprintf("binar header.. %p\n", &_binary_tarfs_start);
  Elf64_Phdr *prog_header = (Elf64_Phdr *)((char *)elf_header + elf_header->e_phoff);

  /* TODO : Remove these prints after development */
#if 0
  if (elf_header) {
    kprintf("ELF.e_type = %d\n", elf_header->e_type);
    kprintf("ELF.e_machine = %d\n", elf_header->e_machine);
    kprintf("ELF.e_version = %d\n", elf_header->e_version);
    kprintf("ELF.e_entry = %d\n", elf_header->e_entry);
    kprintf("ELF.e_phoff = %d\n", elf_header->e_phoff);
    kprintf("ELF.e_shoff = %d\n", elf_header->e_shoff);
    kprintf("ELF.e_flags = %d\n", elf_header->e_flags);
    kprintf("ELF.e_ehsize = %d\n", elf_header->e_ehsize);
    kprintf("ELF.e_phentsize = %d\n", elf_header->e_phentsize);
    kprintf("ELF.e_phnum = %d\n", elf_header->e_phnum);
    kprintf("ELF.e_shentsize = %d\n", elf_header->e_shentsize);
    kprintf("ELF.e_shnum = %d\n", elf_header->e_shnum);
    kprintf("ELF.e_shstrndx = %d\n", elf_header->e_shstrndx);
  }
#endif

  mm_struct_t *mm = task->mm;
  mm->mmap = NULL;

  uint16_t e_phnum = elf_header->e_phnum;
  while (e_phnum) {

    /* Load => Text or Data or BSS segment */
    if (prog_header->p_type == 1) {
#if 0
      kprintf("AMD : start = %p\n", prog_header->p_vaddr);
      kprintf("AMD : end   = %p\n", prog_header->p_vaddr + prog_header->p_memsz);
      uint64_t segment_size = prog_header->p_memsz;
      uint32_t num_pages_required = (segment_size + VIRT_PAGE_SIZE) / VIRT_PAGE_SIZE;
      kprintf("AMD : num pages = %d\n", num_pages_required);
      if (prog_header->p_flags == (PF_R | PF_X))
        kprintf("AMD : Text\n");
      if (prog_header->p_flags == (PF_R | PF_W))
        kprintf("AMD : Data\n");
#endif
      vma_struct_t *vma = (vma_struct_t *)vmm_alloc_page();
      vma->vma_start = prog_header->p_vaddr;
      vma->vma_end = prog_header->p_vaddr + prog_header->p_memsz;
      vma->vma_mm = mm;
      vma->flags = prog_header->p_flags;
      vma->vma_next = NULL;

      if (mm->mmap) {
        vma->vma_next = mm->mmap;
        mm->mmap = vma;
      } else {
        mm->mmap = vma;
      }

      /* Copy segment to program's virtual address space */
      uint64_t v_addr = prog_header->p_vaddr;
      uint64_t segment_size = prog_header->p_memsz;
      uint32_t num_pages_required = (segment_size + VIRT_PAGE_SIZE) / VIRT_PAGE_SIZE;
      while (num_pages_required) {
        /* TODO : v_addr might not be 4K aligned. change if required */
        alloc_segment_mem(v_addr);
        v_addr += VIRT_PAGE_SIZE;
        num_pages_required--;
      }

      kprintf("AMD : start = %p\n", prog_header->p_vaddr);
      kprintf("AMD : end   = %p\n", prog_header->p_vaddr + prog_header->p_memsz);
  //    kprintf("AMD : num pages = %d\n", num_pages_required);
      if (prog_header->p_flags == (PF_R | PF_X))
        kprintf("AMD : Text\n");
      if (prog_header->p_flags == (PF_R | PF_W))
        kprintf("AMD : Data\n");

      if (prog_header->p_flags == (PF_R | PF_X)) {
        /* Text Segment */
        mm->code_start = vma->vma_start;
        mm->code_end   = vma->vma_end;
//        kprintf("tarfs..... %p\n",(&_binary_tarfs_start + prog_header->p_offset));
 //       kprintf("pgm offsett..... %p %p\n",elf_header+ prog_header->p_offset, prog_header->p_filesz);
        
//        alloc_segment_mem((uint64_t)elf_header + prog_header->p_offset);  
//j        memset((void *)0x4000B0, 0, 4096);
  //      while(1);
        memcpy((void *)0x400000, (void *)((uint64_t)elf_header + prog_header->p_offset), prog_header->p_filesz);
        //memcpy((void *)0x4000E8, (void *)0x4000E8, 1);
   //     memcpy((void *)0x400000, (void *)0x400000, 1);
      } else if (prog_header->p_flags == (PF_R | PF_W)) {
        /* Data Segment */
        mm->data_start = vma->vma_start;
        mm->data_end   = vma->vma_end;
      }
    }

    prog_header++;
    e_phnum--;
  }


  task->rip = elf_header->e_entry;
  kprintf("hello .. %p\n",task->rip);
  return 0;
}

