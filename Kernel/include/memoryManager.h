#ifndef memoryManager_h
#define memoryManager_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint64_t size;
    uint64_t used;
    uint64_t free;
} memory_info_t;

typedef struct MemoryManagerCDT* MemoryManagerADT;

void memory_init();
void * memory_alloc(size_t size);
void memory_free(void* ptr);
memory_info_t memory_get_info();

#endif