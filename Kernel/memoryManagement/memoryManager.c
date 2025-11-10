#ifndef USE_BUDDY
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com
#include "memoryManager.h"

typedef struct block_t {
  uint64_t size;
  int is_free;
  struct block_t *next;
} block_t;

#define BLOCK_HEADER_SIZE ((uint64_t)sizeof(block_t))
#define ALIGN4(x) (((((x)-1) >> 2) << 2) + 4)

typedef struct MemoryManagerCDT {
  void *heap_start;
  uint64_t total_size;
  uint64_t used_size;
  block_t *first_block;
} MemoryManagerCDT;

static MemoryManagerCDT memory_manager = {0};
static memory_info_t memory_info = {0};

static block_t *free_list = NULL;

static void update_usage(uint64_t delta, int allocate) {
  if (allocate) {
    memory_manager.used_size += delta;
    if (memory_manager.used_size > memory_manager.total_size) {
      memory_manager.used_size = memory_manager.total_size;
    }
  } else {
    if (delta <= memory_manager.used_size) {
      memory_manager.used_size -= delta;
    } else {
      memory_manager.used_size = 0;
    }
  }

  memory_info.used = memory_manager.used_size;
  memory_info.free = memory_info.size - memory_info.used;
}

void memory_init(void *start, uint64_t size) {

  memory_manager.heap_start = start;
  memory_manager.total_size = size;
  memory_manager.used_size = 0;
  memory_manager.first_block = NULL;

  memory_info.size = size;
  memory_info.used = 0;
  memory_info.free = size;

  free_list = (block_t *)start;
  free_list->size = size - BLOCK_HEADER_SIZE;
  free_list->is_free = 1;
  free_list->next = NULL;

  memory_manager.first_block = free_list;
}

void *memory_alloc(uint64_t size) {
  size = ALIGN4(size);

  block_t *curr = free_list;

  while (curr != NULL) {
    if (curr->is_free && curr->size >= size) {
      if (curr->size >= size + BLOCK_HEADER_SIZE + 4) {
        block_t *new_block =
            (block_t *)((uint8_t *)curr + BLOCK_HEADER_SIZE + size);
        new_block->size = curr->size - size - BLOCK_HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = curr->next;

        curr->size = size;
        curr->next = new_block;
      }

      curr->is_free = 0;
      update_usage(curr->size + BLOCK_HEADER_SIZE, 1);

      return (void *)((uint8_t *)curr + BLOCK_HEADER_SIZE);
    }

    curr = curr->next;
  }

  return NULL;
}

void memory_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  block_t *block_to_free = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
  if (block_to_free->is_free) {
    return;
  }

  block_to_free->is_free = 1;
  update_usage(block_to_free->size + BLOCK_HEADER_SIZE, 0);

  block_t *curr = free_list;
  while (curr != NULL && curr->next != NULL) {
    uint8_t *curr_end = (uint8_t *)curr + BLOCK_HEADER_SIZE + curr->size;
    if (curr->is_free && curr->next->is_free &&
        curr_end == (uint8_t *)curr->next) {
      curr->size += BLOCK_HEADER_SIZE + curr->next->size;
      curr->next = curr->next->next;
    } else {
      curr = curr->next;
    }
  }
}

memory_info_t *memory_get_info() {
  memory_info.free = memory_info.size - memory_info.used;
  return &memory_info;
}

#endif // USE_BUDDY
