#ifndef memoryManager_h
#define memoryManager_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "string.h"

typedef struct {
    uint64_t size;
    uint64_t used;
    uint64_t free;
} memory_info_t;

#define BLOCKSIZE 64
// #define HEAP_SIZE 0x10000000 // esto lo comente recien (soy Jose) creo que se nos fue un 0 demas, aparte no se si lo usamos
#define MEM_SIZE 0x100000



typedef struct MemoryManagerCDT* MemoryManagerADT;

// MemoryManagerADT memory_init(void * const restrict memoryForMemoryManager, uint64_t managedMemory);

void memory_init(void *start, uint64_t size);
void * memory_alloc(uint64_t size);
void memory_free(void* ptr);
memory_info_t memory_get_info();



// MemoryManagerADT memory_init(void *start, uint64_t total_size);

// MemoryManagerADT memory_init(void * start, uint64_t total_size);
// void * memory_alloc(const size_t bytes);

#endif