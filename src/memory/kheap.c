
#include "kheap.h"

memory_block_t* head     = NULL;
memory_block_t* tail     = NULL;
memory_block_t* freeList = NULL;

void* heap_start;
void* heap_end;
void* heap_curr;
void* heap_max;

size_t size_allocated;
size_t heap_size;

bool Kheap_enabled = false;

// Kernel Page Directory
extern page_directory_t * kernel_page_dir;

extern void * heap_start, * heap_end, * heap_max, * heap_curr;

uint32_t placement_address = (uint32_t)&k_end;

void* kmalloc_c(uint32_t size, int align, uint32_t* paddr)
{
    if(align == 1 && (placement_address & 0xFFFFF000))
    {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }

    if(paddr)
    {
        *paddr = placement_address;
    }

    uint32_t tmp = placement_address;
    placement_address += size;
    return (void*)tmp;
}

size_t  get_allocated_size()
{
    return size_allocated;
}

size_t get_heap_size()
{
    return heap_size;
}

size_t get_remaining_size()
{
    return (heap_size-size_allocated);
}

uint32_t kmalloc_int(uint32_t size, int align, uint32_t* paddr)
{
    if(heap_start != NULL)
    {
        if(align == 1)
        {
            size += 4096;
        }
        void* addr = malloc(size);
        uint32_t aligned_addr = ((uint32_t)addr & 0xFFFFF000) + 0x1000;
        if(paddr != 0 )
        {
            uint32_t t = (uint32_t)addr;
            if(align)
                t = aligned_addr;
            *paddr = (uint32_t)virt2phys(kernel_page_dir, (void*)t);
        }

        if(align == 1)
        {
            return aligned_addr;
        }
        else
        {
            return (uint32_t)addr;
        }
    }
    else
    {
        if(align && (placement_address & 0xFFFFF000))
        {
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }

        if(paddr)
        {
            *paddr = placement_address;
        }

        uint32_t tmp = placement_address;
        placement_address += size;
        return tmp;

    }
}

void* kmalloc_a(uint32_t size)
{
    ASM_FUNC("cli");
    void* ret = (void*)kmalloc_int(size, 1, 0);
    ASM_FUNC("sti");
    return ret;
}

uint32_t kmalloc_p(uint32_t size, uint32_t* paddr)
{
    return kmalloc_int(size, 0, paddr);
}

uint32_t kmalloc_ap(uint32_t size, uint32_t* paddr)
{
    return kmalloc_int(size, 1, paddr);
}

void* kmalloc(uint32_t size)
{
    ASM_FUNC("cli");
    void* ret = (void*)kmalloc_int(size, 0, 0);
    ASM_FUNC("sti");
    return ret;
}

void kfree(void* ptr)
{
    ASM_FUNC("cli");
    free(ptr);
    ASM_FUNC("sti");
}

void* krealloc(void* ptr, uint32_t size)
{
    ASM_FUNC("cli");
    void* ret = realloc(ptr, size);
    ASM_FUNC("sti");
    return ret;
}

void debug_kmalloc_print()
{
    printf("Size: %dB\nHeap allocated: %dB\nEnd addr: 0x%06x\nStart addr: 0x%06x\nMax addr: 0x%06x\n", heap_size, size_allocated, heap_start, heap_end, heap_max);
}

void init_kheap(void* start, void* end, void* max)
{
    printf("[KHEAP] Initializing memory allocation...\n");
    heap_start = start;
    heap_end = end;
    heap_max = max;
    heap_curr = start;
    Kheap_enabled = true;
    heap_size = (((uint32_t)end) - ((uint32_t)start));
    serialprintf("[KHEAP] HEAP:\n\t-START: 0x%06x\n\t-END: 0x%06x\n\t-MAX: 0x%06x\n", start, end, max);
    printf("[KHEAP] Kernel Heap Initialized\n");
}

uint32_t getRealSize(uint32_t size)
{
    return (size >> 1) << 1;
}

//void print_db() Will implement

void db_print() {
    if(!head) {serialprintf("your heap is empty now\n");return;}
    //serialprintf("HEAP:\n");
    uint32_t total = 0;
    uint32_t total_overhead = 0;
    memory_block_t * curr = head;
    while(1) {
        // print it
        char c = 'A';
        if(isFree(curr)) c = 'F';
        uint32_t a = getRealSize(curr->size);
        serialprintf("| %u |.%c.| %u |\n", a, c, a);
        total = total + getRealSize(curr->size);
        total_overhead = total_overhead + OVERHEAD;
        if(isEnd(curr)) break;
        void * ptr = (void*)curr + OVERHEAD + getRealSize(curr->size);
        curr = ptr;
    }
    serialprintf("\n total usable bytes: %d", total);
    serialprintf("\n total overhead bytes: %d", total_overhead);
    serialprintf("\n total bytes: %d", total + total_overhead);
    serialprintf("\nfreelist: ");

    memory_block_t * ite = freeList;
    while(ite) {
        serialprintf("(%p)->", ite);
        ite = ite->next;
    }
    serialprintf("\n\n");
    return;
}

int isEnd(memory_block_t* block)
{
    return block == tail;
}

int doesItFit(memory_block_t* block, uint32_t size)
{
    return block->size >= getRealSize(size) && isFree(block);
}

void setFree(uint32_t *size, int x)
{
    if(x)
    {
        *size = *size | 0x1;
        return;
    }
    *size = *size & 0xFFFFFFFE;
}

int isFree(memory_block_t* block)
{
    if(block == NULL)
        return 0;
    return (block->size & 0x1);
}

void removeFromList(memory_block_t* block)
{
    if(!block)
        return;
    if(block->prev)
    {
        block->prev->next = block->next;
        if(block->next)
            block->next->prev = block->prev;
    }
    else
    {
        freeList = block->next;
        if(freeList)
            freeList->prev = NULL;
    }
}

void addToList(memory_block_t* block)
{
    if(!block)
        return;
    block->next = freeList;
    if(freeList)
        freeList->prev = block;
    freeList = block;
    freeList->prev = NULL;
}

memory_block_t* bestFit(uint32_t size)
{
    if(!freeList)
        return NULL;
    memory_block_t* curr = freeList;
    memory_block_t* best = NULL;

    while(curr)
    {
        if(doesItFit(curr, size)) 
        {
            if(best == NULL || curr->size < best->size)
                best = curr;
        }
        if(validate(curr->next) == 1)
            curr = curr->next;
        else
            return NULL;
    }
    return best;
}

memory_block_t* getPrevBlock(memory_block_t* block)
{
    if(block == head)
        return NULL;
    void * ptr = block;
    uint32_t * uptr = ptr - sizeof(uint32_t);
    uint32_t prev_size = getRealSize(*uptr);
    void * ret = ptr - OVERHEAD - prev_size;
    return ret;
}

memory_block_t* getNextBlock(memory_block_t* block)
{
    if(block == tail)
        return NULL;
    void * ptr = block;
    ptr = ptr + OVERHEAD + getRealSize(block->size);
    return ptr;
}

void* malloc(uint32_t size)
{
    if(size == 0)
        return NULL;
    uint32_t roundedSize = ((size + 15)/16) * 16;
    size_allocated += size;
    uint32_t blockSize = roundedSize + OVERHEAD;
    memory_block_t* best;
    best = bestFit(roundedSize);
    uint32_t * trailingSize = NULL;
    if(best)
    {
        void * ptr = (void*)best;
        void * saveNextBlock = getNextBlock(best);
        uint32_t chunkSize = getRealSize(best->size) + OVERHEAD;
        uint32_t rest = chunkSize - blockSize;
        uint32_t whichSize;
        if(rest < 8 + OVERHEAD) whichSize = chunkSize;
        else whichSize = blockSize;
        best->size = whichSize - OVERHEAD;
        setFree(&(best->size), 0);
        void * base = ptr;
        trailingSize = ptr + whichSize - sizeof(uint32_t);
        *trailingSize = best->size;
        ptr = (void*)(trailingSize + 1);
        if(rest < 8 + OVERHEAD)
            goto noSplit;
        if(rest >= 8) 
        {
            if(base != tail && isFree(saveNextBlock)) 
            {
                void * nextblock = saveNextBlock;
                memory_block_t* n_nextblock = nextblock;
                removeFromList(n_nextblock);
                memory_block_t* t = ptr;
                t->size = rest - OVERHEAD + getRealSize(n_nextblock->size) + OVERHEAD;
                setFree(&(t->size), 1);
                ptr = ptr + sizeof(memory_block_t) + getRealSize(t->size);
                trailingSize = ptr;
                *trailingSize = t->size;
                if(nextblock == tail)
                {
                    tail = t;
                }
                addToList(t);
            }
            else
            {
                memory_block_t* putThisBack = ptr;
                putThisBack->size = rest - OVERHEAD;
                setFree(&(putThisBack->size), 1);
                trailingSize = ptr + sizeof(memory_block_t) + getRealSize(putThisBack->size);
                *trailingSize = putThisBack->size;
                if(base == tail)
                {
                    tail = putThisBack;
                }
                addToList(putThisBack);
            }
        }
noSplit:
        removeFromList(base);
        return base + sizeof(memory_block_t);
    }
    else
    {
        uint32_t realsize = blockSize;
        memory_block_t* ret = ksbrk(realsize);
        ASSERT(ret != NULL &&  "[KHEAP] Heap is running out of space\n");
        if(!head) head = ret;
        void * ptr = ret;
        void * save = ret;
        tail = ptr;

        if((validate(ret) != 1))
        {
            serialprintf("[MALLOC] Invalid ksbrk\n");
            ret = ksbrk(realsize);
            if((validate(ret) != 1))
            {
                return NULL;
            }
        }

        ret->size = blockSize - OVERHEAD;
        setFree(&(ret->size), 0);
        ptr = ptr + blockSize - sizeof(uint32_t);
        trailingSize = ptr;
        
        if((validate(ptr) == 1) && (validate(ret + (sizeof(memory_block_t*) * 2)) == 1))
            *trailingSize = ret->size;
        else
            serialprintf("[kmalloc] trailingsize not set\n");

        if(validate(save + sizeof(memory_block_t)) == 1)
            return save + sizeof(memory_block_t);
        else
        {
            serialprintf("[kmalloc] Invalid memory block\n");
            return NULL;
        }
    }
}

int is_from_heap(void* ptr)
{
    memory_block_t* curr = ptr - sizeof(memory_block_t);
    serialprintf("[KHEAP] 0x%x size %dB\n", ptr, curr->size);
    if(validate(curr) == 1 && ((validate(curr->next) == 1 && validate(curr->prev) == 1) || curr->size != 0))
        return 1;

    return 0;
}

void free(void* ptr)
{
    if(validate(ptr) != 1)
        return;
    memory_block_t* curr = ptr - sizeof(memory_block_t);
    if(validate(curr) != 1)
        return;
    memory_block_t* prev = getPrevBlock(curr);
    memory_block_t* next = getNextBlock(curr);
    size_allocated -= curr->size;
    //serialprintf("[DYNMALLOC] PREV: 0x%06x%s | CURR: 0x%06x%s | NEXT: 0x%06x%s\n", (uint32_t)prev, (((uint32_t)prev) > ((uint32_t)heap_max) ? "!" : (((uint32_t)prev) < ((uint32_t)heap_start) ? "!!" : "\0")), (uint32_t)curr, (((uint32_t)curr) > ((uint32_t)heap_max) ? "!" : (((uint32_t)curr) < ((uint32_t)heap_start) ? "!!" : "\0")), (uint32_t)next, (((uint32_t)next) > ((uint32_t)heap_max) ? "!" : (((uint32_t)next) < ((uint32_t)heap_start) ? "!!" : "\0")));
    if(isMemoryPaged(kernel_page_dir, prev) == 0)
    {
        prev = NULL;
        serialprintf("PREVNULL\n");
    }
    if(isMemoryPaged(kernel_page_dir, next) == 0)
    {
        next = NULL;
        serialprintf("NEXTNULL\n");
    }
    if(isFree(prev) && isFree(next))
    {
        prev->size = getRealSize(prev->size) + 2*OVERHEAD + getRealSize(curr->size) + getRealSize(next->size);
        setFree(&(prev->size), 1);
        uint32_t * trailingSize = (void*)prev + sizeof(memory_block_t) + getRealSize(prev->size);
        *trailingSize = prev->size;
        if(tail == next) tail = prev;
        removeFromList(next);
    }
    else if(isFree(prev))
    {
        prev->size = getRealSize(prev->size) + OVERHEAD + getRealSize(curr->size);
        setFree(&(prev->size), 1);
        uint32_t * trailingSize = (void*)prev + sizeof(memory_block_t) + getRealSize(prev->size);
        *trailingSize = prev->size;
        if(tail == curr) tail = prev;
    }
    else if(isFree(next))
    {
        curr->size = getRealSize(curr->size) + OVERHEAD + getRealSize(next->size);
        setFree(&(curr->size), 1);
        uint32_t * trailingSize = (void*)curr + sizeof(memory_block_t) + getRealSize(curr->size);
        *trailingSize = curr->size;
        if(tail == next) tail = curr;
        removeFromList(next);
        addToList(curr);
    }
    else
    {
        setFree(&(curr->size), 1);
        uint32_t * trailingSize = (void*)curr + sizeof(memory_block_t) + getRealSize(curr->size);
        if(validate(trailingSize) == 1)
            *trailingSize = curr->size;
        addToList(curr);
    }
}

void* realloc(void* ptr, uint32_t size)
{
    uint32_t * trailingSize = NULL;
    if(!ptr) return malloc(size);
    if(size == 0 && ptr != NULL)
    {
        free(ptr);
        return NULL;
    }
    uint32_t roundedSize = ((size + 15)/16) * 16;
    uint32_t blockSize = roundedSize + OVERHEAD;
    memory_block_t* nextBlock, * prevBlock;
    memory_block_t* nptr = ptr - sizeof(memory_block_t);
    nextBlock = getNextBlock(nptr);
    prevBlock = getPrevBlock(nptr);
    if(nptr->size == size) return ptr;
    if(nptr->size < size)
    {
        if(tail != nptr && isFree(nextBlock) && (getRealSize(nptr->size) + OVERHEAD + getRealSize(nextBlock->size)) >= roundedSize)
        {
            removeFromList(nextBlock);
            nptr->size = getRealSize(nptr->size) + OVERHEAD + getRealSize(nextBlock->size);
            setFree(&(nptr->size), 0);
            trailingSize = (void*)nptr + sizeof(memory_block_t) + getRealSize(nptr->size);
            *trailingSize = nptr->size;
            if(tail == nextBlock)
            {
                tail = nptr;
            }
            return nptr + 1;
        }
        else if(head != nptr && isFree(prevBlock) && (getRealSize(nptr->size) + OVERHEAD + getRealSize(prevBlock->size)) >= roundedSize)
        {
            uint32_t originalSize = getRealSize(nptr->size);
            removeFromList(prevBlock);
            prevBlock->size = originalSize + OVERHEAD + getRealSize(prevBlock->size);
            setFree(&(prevBlock->size), 0);
            trailingSize = (void*)prevBlock + sizeof(memory_block_t) + getRealSize(prevBlock->size);
            *trailingSize = prevBlock->size;
            if(tail == nptr) {
                tail = prevBlock;
            }
            memcpy(prevBlock+1, ptr, originalSize);
            return prevBlock + 1;
        }
        void * newplace = malloc(size);
        memcpy(newplace, ptr, getRealSize(nptr->size));
        free(ptr);
        return newplace;
    }
    else
    {
        uint32_t rest = getRealSize(nptr->size) + OVERHEAD - blockSize;
        if(rest < 8 + OVERHEAD) return ptr;

        nptr->size = blockSize - OVERHEAD;
        setFree(&(nptr->size), 0);
        trailingSize = (void*)nptr + sizeof(memory_block_t) + getRealSize(nptr->size);
        *trailingSize = nptr->size;
        memory_block_t* splitBlock = (void*)trailingSize + sizeof(uint32_t);

        if(nextBlock && isFree(nextBlock))
        {
            splitBlock->size = rest + getRealSize(nextBlock->size);
            setFree(&(splitBlock->size), 1);
            trailingSize = (void*)splitBlock + sizeof(memory_block_t) + getRealSize(splitBlock->size);
            *trailingSize = splitBlock->size;

            removeFromList(nextBlock);
            if(tail == nextBlock) {
                tail = splitBlock;
            }
            addToList(splitBlock);

            return ptr;
        }
        splitBlock->size = rest - OVERHEAD;
        setFree(&(splitBlock->size), 1);
        trailingSize = (void*) splitBlock + sizeof(memory_block_t) + getRealSize(splitBlock->size);
        *trailingSize = splitBlock->size;
        addToList(splitBlock);

        return ptr;
    }
    
}

void* calloc(uint32_t size, uint32_t num)
{
    void* ptr = malloc(num * size);
    memset(ptr, 0, num * size);
    return ptr;
}

void* zalloc(uint32_t size)
{
    void* ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void* kcalloc(uint32_t size, uint32_t num)
{
    return calloc(num, size);
}