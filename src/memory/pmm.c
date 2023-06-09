#include "pmm.h"

uint8_t* pmm_bitmap = (uint8_t*)(&k_end + 0x1000);
uint8_t* mem_start;
uint32_t total_blocks;
uint32_t pmm_bitmap_size;

uint32_t get_totalBlocks()
{
    return total_blocks;
}

void init_pmm(uint32_t mem_size)
{
    total_blocks = mem_size / BLOCK_SIZE;
    pmm_bitmap_size = total_blocks / BPP;
    if(pmm_bitmap_size * BPP < total_blocks)
        pmm_bitmap_size++;

    memset(pmm_bitmap, 0, pmm_bitmap_size);
    mem_start = (uint8_t*)BLOCK_ALIGN(((uint32_t)(pmm_bitmap + pmm_bitmap_size)));
    printf("[PMM] Physical memory manager initialized.\n");
}

uint32_t alloc_block()
{
    uint32_t free_block = first_free_block();
    SETBIT(free_block);
    return free_block;
}

void free_block(uint32_t block)
{
    CLEARBIT(block);
}

uint32_t first_free_block()
{
    uint32_t i;
    for(i = 0; i < total_blocks; i++) {
        if(!ISSET(i))
            return i;
    }
    printf("[PMM] Ran out of free blocks!\n");
    return (uint32_t) -1;
}
