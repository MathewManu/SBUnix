#include <sys/elf64.h>
#include <sys/task.h>
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/utils.h>


int load_binary(task_struct_t *task, char *bin_filename) {

  Elf64_Ehdr *elf_header = get_elf_header(bin_filename);
  if (elf_header == NULL) {
    return 1;
  }

  Elf64_Phdr *prog_header = (Elf64_Phdr *)((char *)elf_header + elf_header->e_phoff);

  mm_struct_t *mm = task->mm;
  mm->mmap = NULL;

  uint16_t e_phnum = elf_header->e_phnum;
  while (e_phnum) {

    /* Load => Text or Data or BSS segment */
    if (prog_header->p_type == 1) {
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
      uint32_t page_align_offset = v_addr % VIRT_PAGE_SIZE;
      if (page_align_offset) {
        /* v_addr might not be 4K aligned */
        v_addr = v_addr - page_align_offset;
      }
      while (num_pages_required) {
        alloc_segment_mem(v_addr);
        v_addr += VIRT_PAGE_SIZE;
        num_pages_required--;
      }

      if (prog_header->p_flags == (PF_R | PF_X)) {
        /* Text Segment */
        mm->code_start = vma->vma_start;
        mm->code_end   = vma->vma_end;
   	vma->vma_type = VMA_TYPE_TEXT;
        memcpy((void *)prog_header->p_vaddr, (void *)((uint64_t)elf_header + prog_header->p_offset), prog_header->p_filesz);
      } else if (prog_header->p_flags == (PF_R | PF_W)) {
        /* Data Segment */
        mm->data_start = vma->vma_start;
        mm->data_end   = vma->vma_end;
   	vma->vma_type = VMA_TYPE_DATA;
        memcpy((void *)prog_header->p_vaddr, (void *)((uint64_t)elf_header + prog_header->p_offset), prog_header->p_filesz);
      }
    }

    prog_header++;
    e_phnum--;
  }

    alloc_segment_mem(STACK_TOP);  
    uint64_t stk = STACK_TOP;  
    vma_struct_t *vma = (vma_struct_t *)vmm_alloc_page();
    vma->vma_next = NULL;
      if (mm->mmap) {
        vma->vma_next = mm->mmap;
        mm->mmap = vma;
      } else {
        mm->mmap = vma;
      }
    
   vma->vma_start = stk + VIRT_PAGE_SIZE;
   vma->vma_end = stk;
   vma->vma_type = VMA_TYPE_STACK;
   vma->vma_mm = mm;

   task->ursp = stk + 4016; 


   alloc_segment_mem(HEAP_BEGIN);
   uint64_t heap = HEAP_BEGIN;
   vma_struct_t *vma_heap = (vma_struct_t *)vmm_alloc_page();
   vma_heap->vma_next = NULL;
   if (mm->mmap) {
     vma_heap->vma_next = mm->mmap;
     mm->mmap = vma_heap;
   } else {
     mm->mmap = vma_heap;
   }

   vma_heap->vma_start = HEAP_BEGIN;
   vma_heap->vma_end = HEAP_END;
   vma_heap->vma_type = VMA_TYPE_HEAP;
   vma_heap->vma_mm = mm;
   mm->brk = heap;

  task->rip = elf_header->e_entry;
  return 0;
}

