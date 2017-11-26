#ifndef _PMM_H_
#define _PMM_H_

#define MAX_NUM_PHYS_BLOCKS 51200
#define PHYS_BLOCK_SIZE     4096

typedef struct phys_block_t {
    uint8_t used; 
    struct phys_block_t *next;
} phys_block_t;

extern phys_block_t phys_blocks[MAX_NUM_PHYS_BLOCKS];

uint64_t get_phys_mem_end();
uint32_t get_num_free_blocks();
uint32_t get_num_used_blocks();
uint64_t pmm_alloc_block();
void pmm_dealloc_block(uint64_t phys_addr);
void update_phys_blocks(uint64_t start_addr, uint64_t end_addr);
void mark_kernel_blocks(void *physbase, void *physfree);
void init_pmm(uint32_t *modulep, void *physbase, void *physfree);

#endif

