#ifndef __KHEAP_H__
#define __KHEAP_H__

#include "system.h"
#include "pmm.h"
#include "printf.h"
#include "string.h"
#include "paging.h"
#include "debug.h"

#define KHEAP_START (void*)0xC0400000
#define KHEAP_MAX_ADDRESS (void*)0xCFFFFFFF
#define KHEAP_INITIAL_SIZE (KHEAP_MAX_ADDRESS-KHEAP_START)
#define HEAP_MIN_SIZE 4 * MB

#define PAGE_SIZE 4096
#define OVERHEAD (sizeof(struct memory_block) + sizeof(uint32_t))

#define ZALLOC_TYPES(a) zalloc(sizeof(a))

typedef struct memory_block
{
    struct memory_block* next;
    struct memory_block* prev;
    uint32_t size;
} memory_block_t;

extern bool Kheap_enabled;

/*
* if heap is initialized then uss the dynamic memory allocator else use the placement memory allocator
* @param size The size of the memory to be allocated
* @param align Should the memory be aligned?
* @param paddr Should the allocated memory be given a physical address?
*/
uint32_t kmalloc_int(uint32_t size, int align, uint32_t* paddr);

/*
* continuous kmalloc which can be used before the heap is initialised
* @param size The size of the memory to be allocated
* @param align Should the memory be aligned?
* @param paddr Should the allocated memory be given a physical address?
*/
void* kmalloc_c(uint32_t size, int align, uint32_t* paddr);

/*
* Wrapper for kmalloc_int with align = 1
* @param size The size of the memory to be allocated
*/
void* kmalloc_a(uint32_t size);

/*
* Wrapper for kmalloc_int with align = 0 and paddr = paddr
* @param size The size of the memory to be allocated
* @param paddr Should the allocated memory be given a physical address?
*/
uint32_t kmalloc_p(uint32_t size, uint32_t* paddr);

/*
* Wrapper for kmalloc_int with align = 1 and paddr = paddr
* @param size The size of the memory to be allocated
* @param paddr Should the allocated memory be given a physical address?
*/
uint32_t kmalloc_ap(uint32_t size, uint32_t* paddr);

/*
* Wrapper for kmalloc_int with align = 0 and paddr = 0
* @param size The size of the memory to be allocated
*/
void *kmalloc(uint32_t size);

/*
* A function to free a block of memory previously allocated with kmalloc
* @param ptr The pointer to the memory to be freed
*/
void kfree(void* ptr);

/*
* A function to reallocate a block of memory previously allocated with kmalloc
* @param ptr The pointer to the memory to be reallocated
* @param size The size of the memory to be reallocated
*/
void* krealloc(void* ptr, uint32_t size);

void* kcalloc(uint32_t size, uint32_t num);

void* zalloc(uint32_t size);

void print_db();

int doesItFit(memory_block_t* block, uint32_t size);

void setFree(uint32_t *size, int x);

void removeFromList(memory_block_t* block);
void addToList(memory_block_t* block);

int isBetter(memory_block_t* block1, memory_block_t* block2);
memory_block_t* bestFit(uint32_t size);
memory_block_t* getPrevBlock(memory_block_t* block);
memory_block_t* getNextBlock(memory_block_t* block);

uint32_t getRealSize(uint32_t size);
uint32_t getSbrkSize(uint32_t size);

int isFree(memory_block_t* block);
int isEnd(memory_block_t* block);

void init_kheap(void* start, void* end, void* max);

void* malloc(uint32_t size);
void* realloc(void* ptr, uint32_t size);
void* calloc(uint32_t size, uint32_t num);

void free(void* ptr);

#endif /*__KHEAP_H__*/