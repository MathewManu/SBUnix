#include <sys/defs.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/kprintf.h>

#define PAGE_PML4_INDEX(x)            ((x >> 39) & 0x1FF)
#define PAGE_PDP_INDEX(x)             ((x >> 30) & 0x1FF)
#define PAGE_PD_INDEX(x)              ((x >> 21) & 0x1FF)
#define PAGE_PT_INDEX(x)              ((x >> 12) & 0x1FF)
#define PAGE_OFFSET(x)                ( x &  0xFFF)
#define PAGE_GET_PHYSICAL_ADDRESS(x)  (*x & ~0xFFF)

#define PTE_PRESENT   0x1
#define PTE_WRITABLE  0x2
#define PTE_USER      0x4
#define PWU_FLAG      (PTE_PRESENT | PTE_WRITABLE | PTE_USER) /* User Pages */
#define PWS_FLAG      (PTE_PRESENT | PTE_WRITABLE)            /* Supervisor Pages */

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

void page_fault_handler() {

  kprintf("Inside page_fault_handler\n");
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
  uint64_t  set_flags_PWU = (PTE_PRESENT | PTE_WRITABLE | PTE_USER);

  /* Get pml4 entry using virtual address. If not present, create */
  pdp_addr = pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)];
  if (pdp_addr & PTE_PRESENT) {
    pdp = (pdp_t *)PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr);
  } else {
    pdp = (pdp_t *)pmm_alloc_block();
    pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | set_flags_PWU);
  }

  /* Get pdp entry using virtual address. If not present, create */
  pd_addr = pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)];
  if (pd_addr & PTE_PRESENT) {
    pd = (pd_t *)PAGE_GET_PHYSICAL_ADDRESS(&pd_addr);
  } else {
    pd = (pd_t *)pmm_alloc_block();
    pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | set_flags_PWU);
  }
   
  /* Get pd entry using virtual address. If not present, create */
  pt_addr = pd->pd_entries[PAGE_PD_INDEX(v_addr)];
  if (pt_addr & PTE_PRESENT) {
    pt = (pt_t *)PAGE_GET_PHYSICAL_ADDRESS(&pt_addr);
  } else {
    pt = (pt_t *)pmm_alloc_block();
    pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | set_flags_PWU);
  }

  /* Update pt entry with the provided physical address */
  pt->pt_entries[PAGE_PT_INDEX(v_addr)] = p_addr | set_flags_PWU; 
}

void virt_phys_map(pml4_t *pml4, uint64_t v_addr, uint64_t p_addr) {

  pml4 = (pml4_t *)((uint64_t)pml4 | VIRT_ADDR_BASE);
  pdp_t   *pdp; 
  pd_t    *pd; 
  pt_t    *pt; 

  uint64_t  pdp_addr; 
  uint64_t  pd_addr; 
  uint64_t  pt_addr; 
  uint64_t  set_flags_PWU = (PTE_PRESENT | PTE_WRITABLE | PTE_USER);

  /* Get pml4 entry using virtual address. If not present, create */
  pdp_addr = pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)];
  if (pdp_addr & PTE_PRESENT) {
    pdp = (pdp_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pdp_addr));
  } else {
    pdp = (pdp_t *)(pmm_alloc_block());
    pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | set_flags_PWU);
  }

  /* Get pdp entry using virtual address. If not present, create */
  pdp = (pdp_t *)((uint64_t)pdp | VIRT_ADDR_BASE);
  pd_addr = pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)];
  if (pd_addr & PTE_PRESENT) {
    pd = (pd_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pd_addr));
  } else {
    pd = (pd_t *)(pmm_alloc_block());
    pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | set_flags_PWU);
  }
   
  /* Get pd entry using virtual address. If not present, create */
  pd = (pd_t *)((uint64_t)pd | VIRT_ADDR_BASE);
  pt_addr = pd->pd_entries[PAGE_PD_INDEX(v_addr)];
  if (pt_addr & PTE_PRESENT) {
    pt = (pt_t *)(PAGE_GET_PHYSICAL_ADDRESS(&pt_addr));
  } else {
    pt = (pt_t *)(pmm_alloc_block());
    pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | set_flags_PWU);
  }

  /* Update pt entry with the provided physical address */
  pt = (pt_t *)((uint64_t)pt | VIRT_ADDR_BASE);
  pt->pt_entries[PAGE_PT_INDEX(v_addr)] = p_addr | set_flags_PWU; 
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
  uint64_t  set_flags_PWU = (PTE_PRESENT | PTE_WRITABLE | PTE_USER);

  /* Allocate memory for pml4 table */
  pml4 = (pml4_t *)pmm_alloc_block();

  /* Allocate and insert pdp table entry in pml4 table */
  pdp = (pdp_t *)pmm_alloc_block();
  pml4->pml4_entries[PAGE_PML4_INDEX(v_addr)] = ((uint64_t)pdp | set_flags_PWU);
  
  /* Allocate and insert page directory entry in pdp table */
  pd = (pd_t *)pmm_alloc_block();
  pdp->pdp_entries[PAGE_PDP_INDEX(v_addr)] = ((uint64_t)pd | set_flags_PWU);

  /* Allocate and insert page table entry in page directory */
  pt = (pt_t *)pmm_alloc_block();
  pd->pd_entries[PAGE_PD_INDEX(v_addr)] = ((uint64_t)pt | set_flags_PWU);

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

/* Return the address of a page(virtual address)
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
  }

  return v_addr;
}

void alloc_segment_mem(uint64_t v_addr) {
  uint64_t  p_addr = pmm_alloc_block();
  if (p_addr != -1) {
    virt_phys_map(get_cr3(), v_addr, p_addr);

  } else {
    /* No more physical memory available */
    kprintf("No Memory Available !!!\n");
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

