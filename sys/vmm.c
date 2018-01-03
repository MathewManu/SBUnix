#include <sys/defs.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/utils.h>
#include <sys/kprintf.h>
#include <sys/task.h>

#define PAGE_PML4_INDEX(x)            ((x >> 39) & 0x1FF)
#define PAGE_PDP_INDEX(x)             ((x >> 30) & 0x1FF)
#define PAGE_PD_INDEX(x)              ((x >> 21) & 0x1FF)
#define PAGE_PT_INDEX(x)              ((x >> 12) & 0x1FF)
#define PAGE_OFFSET(x)                ( x &  0xFFF)
#define PAGE_GET_PHYSICAL_ADDRESS(x)  (*x & ~0xFFF)

#define PTE_PRESENT   0x1
#define PTE_WRITABLE  0x2
#define PTE_USER      0x4
#define PTE_COW       0x100
#define PWU_FLAG      (PTE_PRESENT | PTE_WRITABLE | PTE_USER) /* User Pages */
#define PUC_FLAG      (PTE_PRESENT | PTE_USER     | PTE_COW)  /* User Pages with COW */
#define PWS_FLAG      (PTE_PRESENT | PTE_WRITABLE)            /* Supervisor Pages */

#define INVALID_ADDRESS   (~0)
#define MARK_PAGE_ABSENT  1

extern char kernmem;

/* Virtual address to be returned for the next page alloc request after validation */
uint64_t virt_addr = VIRT_ADDR_BASE;

uint8_t is_paging_enabled = 0;

pml4_t *pml4;

uint8_t get_is_paging_enabled() {
  return is_paging_enabled;
}

void set_is_paging_enabled() {
  is_paging_enabled = 1;
}

void set_cr3(pml4_t *pml4) {
  __asm__ volatile("movq %0, %%cr3"::"r"(pml4));
}

pml4_t *get_cr3() {
  pml4_t *current_cr3;
  __asm__ volatile("mov %%cr3, %0":"=r"(current_cr3));

  return current_cr3;
}

static uint64_t get_cr2() {
  uint64_t cr2;
  __asm__ volatile("mov %%cr2, %0":"=r"(cr2));

  return cr2;
}

pml4_t *get_kernel_pml4() {
  return pml4;
}

/*
 * Enable paging by updating cr0
 */
void enable_paging() {
  uint64_t cr0;
  __asm__ volatile("mov %%cr0, %0":"=r"(cr0));
  cr0 |= 0x80000000; /* Enable paging */
  __asm__ volatile("mov %0, %%cr0"::"r"(cr0));

  set_is_paging_enabled();
}


/*
 * Returns the physical address corresponding to the given virtual address
 * Also marks the page as absent if mark_as_absent is set
 */
uint64_t get_physical_addr(pml4_t *pml4, uint64_t v_addr, uint8_t mark_as_absent) {

  pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pdp_t   *pdp;
  pd_t    *pd;
  pt_t    *pt;

  uint64_t  pdp_addr;
  uint64_t  pd_addr;
  uint64_t  pt_addr;

  /* Get pml4 entry using virtual address */
  pdp_addr = pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)];
  if (pdp_addr & PTE_PRESENT) {
    pdp = (pdp_t *)PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr);
  } else {
    return INVALID_ADDRESS;
  }

  /* Get pdp entry using virtual address */
  pdp = (pdp_t *)((uint64_t)pdp | VIRT_ADDR_BASE);
  pd_addr = pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)];
  if (pd_addr & PTE_PRESENT) {
    pd = (pd_t *)PAGE_GET_PHYSICAL_ADDRESS(&pd_addr);
  } else {
    return INVALID_ADDRESS;
  }

  /* Get pd entry using virtual address */
  pd = (pd_t *)((uint64_t)pd | VIRT_ADDR_BASE);
  pt_addr = pd->pd_entries[PAGE_PD_INDEX(v_addr)];
  if (pt_addr & PTE_PRESENT) {
    pt = (pt_t *)PAGE_GET_PHYSICAL_ADDRESS(&pt_addr);
  } else {
    return INVALID_ADDRESS;
  }

  pt = (pt_t *)((uint64_t)pt | VIRT_ADDR_BASE);
  if (mark_as_absent) {
    /* Set as absent */
    pt->pt_entries[PAGE_PT_INDEX(v_addr)] &= ~PTE_PRESENT;
  }

  /* Return the stored physical address. Last 3 bits are flags and hence ignored */
  //return PAGE_GET_PHYSICAL_ADDRESS(&(pt->pt_entries[PAGE_PT_INDEX(v_addr)]));
  return (pt->pt_entries[PAGE_PT_INDEX(v_addr)]);
}

/*
 * Map the given virtual and physical addresses
 * PDP, PD and PT entries corresponding to the virtual address
 * are created if they are not already present
 */
void virt_phys_map_before_paging(pml4_t *pml4, uint64_t v_addr, uint64_t p_addr) {

  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  uint64_t  pdp_addr; 
  uint64_t  pd_addr; 
  uint64_t  pt_addr; 

  /* Get pml4 entry using virtual address. If not present, create */
  pdp_addr = pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)];
  if (pdp_addr & PTE_PRESENT) {
    pdp = (pdp_t *)PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr);
  } else {
    pdp = (pdp_t *)pmm_alloc_block();
    pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | PWU_FLAG);
  }

  /* Get pdp entry using virtual address. If not present, create */
  pd_addr = pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)];
  if (pd_addr & PTE_PRESENT) {
    pd = (pd_t *)PAGE_GET_PHYSICAL_ADDRESS(&pd_addr);
  } else {
    pd = (pd_t *)pmm_alloc_block();
    pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | PWU_FLAG);
  }
   
  /* Get pd entry using virtual address. If not present, create */
  pt_addr = pd->pd_entries[PAGE_PD_INDEX(v_addr)];
  if (pt_addr & PTE_PRESENT) {
    pt = (pt_t *)PAGE_GET_PHYSICAL_ADDRESS(&pt_addr);
  } else {
    pt = (pt_t *)pmm_alloc_block();
    pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | PWU_FLAG);
  }

  /* Update pt entry with the provided physical address */
  pt->pt_entries[PAGE_PT_INDEX(v_addr)] = p_addr | PWU_FLAG; 
}

void virt_phys_map(pml4_t *pml4, uint64_t v_addr, uint64_t p_addr) {

  pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  uint64_t  pdp_addr; 
  uint64_t  pd_addr; 
  uint64_t  pt_addr; 

  /* Get pml4 entry using virtual address. If not present, create */
  pdp_addr = pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)];
  if (pdp_addr & PTE_PRESENT) {
    pdp = (pdp_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr));
  } else {
    pdp = (pdp_t *)(pmm_alloc_block());
    pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | PWU_FLAG);
  }

  /* Get pdp entry using virtual address. If not present, create */
  pdp = (pdp_t *)((uint64_t)pdp | VIRT_ADDR_BASE);
  pd_addr = pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)];
  if (pd_addr & PTE_PRESENT) {
    pd = (pd_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pd_addr));
  } else {
    pd = (pd_t *)(pmm_alloc_block());
    pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | PWU_FLAG);
  }
   
  /* Get pd entry using virtual address. If not present, create */
  pd = (pd_t *)((uint64_t)pd | VIRT_ADDR_BASE);
  pt_addr = pd->pd_entries[PAGE_PD_INDEX(v_addr)];
  if (pt_addr & PTE_PRESENT) {
    pt = (pt_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pt_addr));
  } else {
    pt = (pt_t *)(pmm_alloc_block());
    pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | PWU_FLAG);
  }

  /* Update pt entry with the provided physical address */
  pt = (pt_t *)((uint64_t)pt | VIRT_ADDR_BASE);
  pt->pt_entries[PAGE_PT_INDEX(v_addr)] = p_addr | PWU_FLAG; 
}

void create_video_memory_map() {

  virt_phys_map_before_paging(pml4, VIDEO_VIRT_MEM_BEGIN, VIDEO_PHYS_MEM_BEGIN);
}

/* Initialize IA-32 paging mechanism */
void init_paging(uint64_t physbase, uint64_t physfree) {

  setup_four_level_paging(physbase, physfree);

  /* Video memory virtual address to physical address mapping */
  create_video_memory_map();

  identity_mapping();

  set_cr3(pml4);

  /* Lets's do it */
  enable_paging();
}

/*
 * Allocate the initial four level
 * paging tables. PML4, PDP, PD and PT
 * Then remap kernel to higher range address
 */
void setup_four_level_paging(uint64_t physbase, uint64_t physfree) {

  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  uint64_t  v_addr = (uint64_t)&kernmem;

  /* Allocate memory for pml4 table */
  pml4 = (pml4_t *)pmm_alloc_block();

  /* Allocate and insert pdp table entry in pml4 table */
  pdp = (pdp_t *)pmm_alloc_block();
  pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | PWU_FLAG);
  
  /* Allocate and insert page directory entry in pdp table */
  pd = (pd_t *)pmm_alloc_block();
  pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | PWU_FLAG);

  /* Allocate and insert page table entry in page directory */
  pt = (pt_t *)pmm_alloc_block();
  pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | PWU_FLAG);

  remap_kernel(pt, physbase, physfree);
}

/*
 * Remapping kernel to higher order address
 * starting from physbase - physfree
 */
void remap_kernel(pt_t *pt, uint64_t p_base, uint64_t p_free){

  uint64_t  v_addr = (uint64_t)&kernmem;
  
  while (p_base < p_free) {
    pt->pt_entries[PAGE_PT_INDEX(v_addr)] = p_base | PWU_FLAG; /* TODO : Need to use PWS_FLAG */
    p_base += PHYS_BLOCK_SIZE;
    v_addr += VIRT_PAGE_SIZE;
  }
}

/*
 * Returns the address of a page(virtual address)
 */
uint64_t vmm_alloc_page() {

  uint64_t v_addr = -1;
  uint64_t p_addr = pmm_alloc_block();
  if (p_addr != -1) {
    v_addr = virt_addr;
    virt_phys_map(get_cr3(), v_addr, p_addr);
    virt_addr += VIRT_PAGE_SIZE;

  } else {
    /* No more physical memory available */
    kprintf("No Memory Available !!!\n");
    while(1);
  }

  return v_addr;
}

/*
 * Deallocate a previously allocated page
 */
void vmm_dealloc_page(uint64_t v_addr) {

  uint64_t p_addr = get_physical_addr(get_cr3(), v_addr, MARK_PAGE_ABSENT);
  if (p_addr != INVALID_ADDRESS) {
    pmm_dealloc_block(p_addr);
  }
}

void alloc_segment_mem(uint64_t v_addr) {
  uint64_t  p_addr = pmm_alloc_block();
  if (p_addr != -1) {
    virt_phys_map(get_cr3(), v_addr, p_addr);

  } else {
    /* No more physical memory available */
    kprintf("No Memory Available !!!\n");
    while(1);
  }
}

void identity_mapping() {

	uint64_t p_addr = 0x0;
	uint64_t v_addr = VIRT_ADDR_BASE;
	while (p_addr < get_phys_mem_end()) {

  	virt_phys_map_before_paging(pml4, v_addr, p_addr);
		v_addr += VIRT_PAGE_SIZE;
		p_addr += PHYS_BLOCK_SIZE;
	}
}

void page_fault_handler(uint64_t error_code) {

  /* TODO : Enhance the page fault handling and remove print statements */
  //uint64_t error_code;

  //__asm__ volatile("movq 136(%%rsp), %0":"=r"(error_code));
  //__asm__ volatile("movq 128(%%rsp), %0":"=r"(error_code));

  uint64_t fault_addr_pte;
  uint64_t nw_vaddr;

  uint64_t faulting_address = get_cr2();



  if(error_code & PTE_PRESENT && error_code & PTE_WRITABLE) {

    fault_addr_pte = get_physical_addr(get_cr3(), faulting_address, 0);

    /* COW handling*/
    if(fault_addr_pte & PTE_COW) {

      nw_vaddr = vmm_alloc_page();

      uint64_t phy_addr = get_physical_addr(get_cr3(), PAGE_GET_PHYSICAL_ADDRESS(&nw_vaddr), 0);

      memcpy((void*)nw_vaddr, (void*)(faulting_address & ~0xFFF), VIRT_PAGE_SIZE);

      virt_phys_map(get_cr3(), (faulting_address & ~0xFFF), phy_addr);

      //kprintf("COW Page Fault : addr = %p, fault_addr_pte = %x, phy_addr = %x error_code = %x\n", faulting_address, fault_addr_pte, phy_addr, error_code);
      return;

    }
  }
  //kprintf("Page Fault : addr = %p, error_code = 0x%x\n", faulting_address, error_code);
  /* autogrow stack */
  int stack_grow = 0;
  vma_struct_t *vma = get_current_running_task()->mm->mmap;
  while(vma) {
   if (vma->vma_type == VMA_TYPE_STACK) {
	  if (faulting_address < vma->vma_end && (vma->vma_end - VIRT_PAGE_SIZE) >= MAX_STACK_SIZE) {
	    alloc_segment_mem(faulting_address - VIRT_PAGE_SIZE);
	    //kprintf("*****************8inside autogrow\n");
	    vma->vma_end = faulting_address - VIRT_PAGE_SIZE;
	    stack_grow = 1;	
	    break;
	  }
   }

   vma = vma->vma_next;
  }

  if(!stack_grow) {
	  alloc_segment_mem(faulting_address);
  }

}

/* Function to create page table for child process */
void create_child_paging(uint64_t child_task_pml4) {

  pml4_t  *child_pml4  = (pml4_t *)((uint64_t)child_task_pml4 | VIRT_ADDR_BASE);
  pml4_t  *parent_pml4 = (pml4_t *)((uint64_t)get_cr3() | VIRT_ADDR_BASE);
  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  uint64_t  pdp_addr; 
  uint64_t  pd_addr; 
  uint64_t  pt_addr; 
  uint64_t  pt_entry; 

  /* PML4 Level */
  int pml4_indx = 0;
  while (pml4_indx < 511) {
    pdp_addr = parent_pml4->pml4_entries[pml4_indx];
    if (pdp_addr & PTE_PRESENT) {
      pdp_t *child_pdp = (pdp_t *)(pmm_alloc_block());
      child_pml4->pml4_entries[pml4_indx] = ((uint64_t)child_pdp | PWU_FLAG);
      child_pdp = (pdp_t *)((uint64_t)child_pdp | VIRT_ADDR_BASE);              

      pdp = (pdp_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr));
      pdp = (pdp_t *)((uint64_t)pdp | VIRT_ADDR_BASE);

      /* PDP Level */
      int pdp_indx = 0;
      while (pdp_indx < 512) {
        pd_addr = pdp->pdp_entries[pdp_indx];
        if (pd_addr & PTE_PRESENT) {
          pd_t *child_pd = (pd_t *)(pmm_alloc_block());
          child_pdp->pdp_entries[pdp_indx] = ((uint64_t)child_pd | PWU_FLAG);
	        child_pd = (pd_t *)((uint64_t)child_pd | VIRT_ADDR_BASE);

          pd = (pd_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pd_addr));
          pd = (pd_t *)((uint64_t)pd | VIRT_ADDR_BASE);

          /* PD Level */
          int pd_indx = 0;
          while (pd_indx < 512) {
            pt_addr = pd->pd_entries[pd_indx];
            if (pt_addr & PTE_PRESENT) {
              pt_t *child_pt = (pt_t *)(pmm_alloc_block());
              child_pd->pd_entries[pd_indx] = ((uint64_t)child_pt | PWU_FLAG);

              pt = (pt_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pt_addr));
              pt = (pt_t *)((uint64_t)pt | VIRT_ADDR_BASE);

              /* PT Level */
              int pt_indx = 0;
              while (pt_indx < 512) {
                pt_entry = pt->pt_entries[pt_indx];
                if(pt_entry & PTE_PRESENT) {

                  uint64_t pt_page = (PAGE_GET_PHYSICAL_ADDRESS(&pt_entry));
                  //pt_entry = pt_page | PWU_FLAG;	
                  pt_entry = pt_page | PUC_FLAG;	

                  child_pt = (pt_t *)((uint64_t)child_pt | VIRT_ADDR_BASE);              
                  child_pt->pt_entries[pt_indx] = pt_entry;
                //  pt_entry = pt_page | PWU_FLAG;	
                  pt_entry = pt_page | PUC_FLAG;	
                  pt->pt_entries[pt_indx] = pt_entry;	
                }	

                pt_indx++;
              }
            }
            pd_indx++;
          }
        }
        pdp_indx++;
      }
    }
    pml4_indx++;
  }

  child_pml4->pml4_entries[511] = parent_pml4->pml4_entries[511];
  //child_pml4->pml4_entries[510] = parent_pml4->pml4_entries[510];
}

void cleanup_page_table(pml4_t *pml4) {

  pml4_t  *tmp_pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  pdp_t   *pdp_phys; 
  pd_t    *pd_phys; 
  pt_t    *pt_phys; 

  uint64_t  pdp_addr; 
  uint64_t  pd_addr; 
  uint64_t  pt_addr; 

  /* PML4 Level */
  int pml4_indx = 0;
  while (pml4_indx < 511) {
    pdp_addr = tmp_pml4->pml4_entries[pml4_indx];
    if (pdp_addr & PTE_PRESENT) {
      pdp = (pdp_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr));
      pdp_phys = pdp;
      pdp = (pdp_t *)((uint64_t)pdp | VIRT_ADDR_BASE);
  
      /* PDP Level */
      int pdp_indx = 0;
      while (pdp_indx < 512) {
        pd_addr = pdp->pdp_entries[pdp_indx];
        if (pd_addr & PTE_PRESENT) {
          pd = (pd_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pd_addr));
          pd_phys = pd;
          pd = (pd_t *)((uint64_t)pd | VIRT_ADDR_BASE);

          /* PD Level */
          int pd_indx = 0;
          while (pd_indx < 512) {
            pt_addr = pd->pd_entries[pd_indx];
            if (pt_addr & PTE_PRESENT) {
              pt = (pt_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pt_addr));
              pt_phys = pt;
              pt = (pt_t *)((uint64_t)pt | VIRT_ADDR_BASE);

              pmm_dealloc_block((uint64_t)pt_phys);
            }

            pd_indx++;
          }

          pmm_dealloc_block((uint64_t)pd_phys);
        }

        pdp_indx++;
      }

      pmm_dealloc_block((uint64_t)pdp_phys);
    }

    pml4_indx++;
  }

  pmm_dealloc_block((uint64_t)pml4);
}

