#include <sys/defs.h>
#include <sys/pmm.h>
#include <sys/vmm.h>
#include <sys/kprintf.h>
#include <sys/utils.h>

phys_block_t *free_list = NULL;
phys_block_t *used_list = NULL;
phys_block_t *free_list_tail = NULL; /* Connect e820 returned phys mem chunks */

uint64_t phys_mem_end;

uint64_t get_phys_mem_end() {
  return phys_mem_end;
}

/* Stats : Return the number of physical memory bloks in the list */
static uint32_t get_num_blocks(phys_block_t *list) {

  uint32_t num_phys_blocks = 0;
  phys_block_t *lst = list;

  /* Iterate through the list and increment count */ 
  while (lst) {
    num_phys_blocks++;
    lst = lst->next;
  }

  return num_phys_blocks;
}

/* Stats : Return the number of free physical memory blocks */
uint32_t get_num_free_blocks() {

  return get_num_blocks(free_list);
}

/* Stats : Return the number of used physical memory blocks */
uint32_t get_num_used_blocks() {

  return get_num_blocks(used_list);
}

/*
 * Allocate a block(size = PHYS_BLOCK_SIZE) of physical memory
 */
uint64_t pmm_alloc_block() {

  uint32_t block_index; 
  uint64_t phys_addr;

  /* free_block points to the physical block that is to be returned */
  phys_block_t *free_block = free_list;
  if (!free_block) {
    return -1;
  }

  /* Update free_list to next item in the free_list and move the 
   * allocated physical block to the front of used_list
   */
  free_list = free_list->next;
  free_block->next = used_list;
  free_block->used = 1;
  used_list = free_block;

  /* Getting the index of free block using array base address */
  block_index = free_block - phys_blocks;
  phys_addr = (uint64_t)(block_index * PHYS_BLOCK_SIZE);

  /* Clean the physical block of memory */
  if (get_is_paging_enabled()) 
    memset((void *)(phys_addr | VIRT_ADDR_BASE), 0, PHYS_BLOCK_SIZE);
  else
    memset((void *)phys_addr, 0, PHYS_BLOCK_SIZE);
  
  return phys_addr;
}

/*
 * Deallocate the memory, i.e remove this block of
 * physical memory from the used_list and add to the free_list
 */
void pmm_dealloc_block(uint64_t phys_addr) {

  uint32_t index = phys_addr / PHYS_BLOCK_SIZE;
 
  /* Block is at the beginning of the used_list */
  if (used_list == &phys_blocks[index]) {

    /* Update used_list and move the block to the front of free_list */
    used_list = used_list->next;
    phys_blocks[index].next = free_list;
    phys_blocks[index].used = 0;
    free_list = &phys_blocks[index];

  } else {

    /* Block is not at the beginning of the used_list, so find it */
    phys_block_t *tmp = used_list;
    while (tmp->next != &phys_blocks[index]) {
      tmp = tmp->next;
    }

    /* Add the block to the beginning of the free_list */
    tmp->next = phys_blocks[index].next;
    phys_blocks[index].next = free_list;
    phys_blocks[index].used = 0;
    free_list = &phys_blocks[index];
  }
}

/*
 * update phys_blocks[] using start_addr to end_addr.
 * update freelist for memory chunks reported e820
 * http://wiki.osdev.org/Page_Frame_Allocation Hybrid scheme 2
 */
void update_phys_blocks(uint64_t start_addr, uint64_t end_addr) {

  uint32_t i = 0;
  uint32_t start_index = start_addr / PHYS_BLOCK_SIZE;
  uint32_t end_index = end_addr / PHYS_BLOCK_SIZE;
  if (end_addr && !(end_addr % PHYS_BLOCK_SIZE))
    end_index--;

  /* free_list_tail is used for connecting the chunks of physical memory
   * Not applicable for the very first chunk of physical memory
   */
  if (free_list_tail) {
    free_list_tail->next = &phys_blocks[start_index];
  }

  i = start_index;
  while (i < end_index) {

    phys_blocks[i].next = &phys_blocks[i + 1];

    /* set head pointer */
    if (!free_list) {
      free_list = &phys_blocks[i];
    }
    i++;
  }

  phys_blocks[i].next = NULL;
  free_list_tail = &phys_blocks[i];
}

/*
 * marking blocks from physbase to physfree as used since 
 * kernel & related data reside there
 */
void mark_kernel_blocks(void *physbase, void *physfree) {
  
  uint32_t i = 0;
  uint32_t start_index = (uint64_t)physbase / PHYS_BLOCK_SIZE;
  uint32_t end_index = (uint64_t)physfree / PHYS_BLOCK_SIZE;
  if ((uint64_t)physfree && !((uint64_t)physfree % PHYS_BLOCK_SIZE))
    end_index--;

  /* TODO : Case when i == start_index, this function is not used for now  */ 
  while (phys_blocks[i].next != &phys_blocks[start_index]) {
    i++;
  }

  phys_blocks[i].next = phys_blocks[end_index].next;
  phys_blocks[end_index].next = NULL;
  used_list = &phys_blocks[start_index];
}

/*
 * marking blocks from 0 to physfree as used since 
 * kernel & related data reside there
 */
void mark_blocks_used(void *physfree) {
  
  uint32_t end_index = (uint64_t)physfree / PHYS_BLOCK_SIZE;
  if ((uint64_t)physfree && !((uint64_t)physfree % PHYS_BLOCK_SIZE))
    end_index--;

  /* Move the intitial blocks (till physfree) to used_list */
  int i = 0;
  phys_block_t *tmp = free_list;
  while (tmp && i < end_index) {
    
    tmp = tmp->next;
    i++;
  }

  used_list = free_list;
  free_list = tmp->next;
  tmp->next = NULL;
}

/*
 * initialize physical memory manager.
 */
void init_pmm(uint32_t *modulep, void *physbase, void *physfree) {

  struct smap_t {
    uint64_t base, length;
    uint32_t type;
  }__attribute__((packed)) *smap;

  while(modulep[0] != 0x9001) modulep += modulep[1]+2;
  int i =0;
  for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
    if (smap->type == 1 /* memory */ && smap->length != 0) {
		  i++;	
      kprintf("Available Physical Memory [%p - %p]\n", smap->base, smap->base + smap->length);
      update_phys_blocks(smap->base, smap->base + smap->length);
      phys_mem_end = smap->base + smap->length;
    }
  }

  /* Mark the initial physical blocks of memory as used (kernel and related data) */ 
  mark_blocks_used(physfree); 
}

