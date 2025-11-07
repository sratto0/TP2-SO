#ifndef memoryManager_h
#define memoryManager_h

#include "../../SharedLibraries/sharedStructs.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define BLOCKSIZE 64
#define MEM_SIZE 0x100000

typedef struct MemoryManagerCDT *MemoryManagerADT;

void memory_init(void *start, uint64_t size);
void *memory_alloc(uint64_t size);
void memory_free(void *ptr);
memory_info_t *memory_get_info();

#endif