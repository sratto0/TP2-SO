#ifdef USE_BUDDY

#include "memoryManager.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_POSSIBLE_ORDER 32
#define MIN_BLOCK_SIZE 32U
#define MIN_ORDER 5U

typedef struct buddy_block {
  struct buddy_block *next;
  struct buddy_block *prev;
} buddy_block_t;

typedef struct {
  uint32_t order;
  uint32_t magic;
} alloc_header_t;

#define ALLOC_MAGIC 0xDEADBEEF
#define HEADER_SIZE ((uint32_t)sizeof(alloc_header_t))

static buddy_block_t *free_lists[MAX_POSSIBLE_ORDER + 1] = {0};
static uint32_t max_order = 0;
static void *heap_start = NULL;
static uint64_t heap_size = 0;
static memory_info_t mem_info = {0};

static inline uint32_t log2_ceil(uint64_t n) {
  uint32_t order = 0;
  uint64_t size = 1ULL;
  while (size < n && order < MAX_POSSIBLE_ORDER) {
    size <<= 1ULL;
    order++;
  }
  return order;
}

static inline uintptr_t get_buddy_offset(void *block, uint32_t order) {
  uintptr_t offset = (uintptr_t)block - (uintptr_t)heap_start;
  return offset ^ (uintptr_t)(1ULL << order);
}

static inline void *get_buddy_address(void *block, uint32_t order) {
  uintptr_t buddy_offset = get_buddy_offset(block, order);
  return (void *)((uintptr_t)heap_start + buddy_offset);
}

static void add_to_free_list(void *block, uint32_t order) {
  if (order > MAX_POSSIBLE_ORDER || block == NULL) {
    return;
  }

  buddy_block_t *buddy_block = (buddy_block_t *)block;
  buddy_block->next = free_lists[order];
  buddy_block->prev = NULL;

  if (free_lists[order] != NULL) {
    free_lists[order]->prev = buddy_block;
  }
  free_lists[order] = buddy_block;
}

static void remove_from_free_list(void *block, uint32_t order) {
  if (block == NULL || order > MAX_POSSIBLE_ORDER) {
    return;
  }
  buddy_block_t *buddy_block = (buddy_block_t *)block;

  if (buddy_block->prev != NULL) {
    buddy_block->prev->next = buddy_block->next;
  } else {
    free_lists[order] = buddy_block->next;
  }

  if (buddy_block->next != NULL) {
    buddy_block->next->prev = buddy_block->prev;
  }
}

static void *find_and_remove_buddy(void *block, uint32_t order) {
  if (order > MAX_POSSIBLE_ORDER) {
    return NULL;
  }

  void *buddy = get_buddy_address(block, order);
  buddy_block_t *curr = free_lists[order];

  while (curr != NULL) {
    if ((void *)curr == buddy) {
      remove_from_free_list(buddy, order);
      return buddy;
    }
    curr = curr->next;
  }
  return NULL;
}

static void split_block(void *block, uint32_t order) {
  if (order <= MIN_ORDER) {
    return;
  }

  uint32_t new_order = order - 1U;
  uint64_t block_size = 1ULL << new_order;

  void *buddy = (void *)((uint8_t *)block + block_size);

  add_to_free_list(block, new_order);
  add_to_free_list(buddy, new_order);
}

void memory_init(void *start, uint64_t size) {
  if (start == NULL || size < MIN_BLOCK_SIZE) {
    heap_start = NULL;
    heap_size = 0;
    mem_info.size = 0;
    mem_info.used = 0;
    mem_info.free = 0;
    return;
  }

  heap_start = start;
  heap_size = size;

  mem_info.size = size;
  mem_info.used = 0;
  mem_info.free = size;

  max_order = log2_ceil(size);
  if (max_order > MAX_POSSIBLE_ORDER) {
    max_order = MAX_POSSIBLE_ORDER;
  }
  while ((1ULL << max_order) > size && max_order > 0) {
    max_order--;
  }
  if (max_order < MIN_ORDER) {
    max_order = MIN_ORDER;
  }

  for (uint32_t i = 0; i <= MAX_POSSIBLE_ORDER; i++) {
    free_lists[i] = NULL;
  }

  add_to_free_list(heap_start, max_order);
}

void *memory_alloc(uint64_t size) {
  if (heap_start == NULL || size == 0) {
    return NULL;
  }

  uint64_t total_size = size + HEADER_SIZE;
  if (total_size < MIN_BLOCK_SIZE) {
    total_size = MIN_BLOCK_SIZE;
  }

  uint32_t order = log2_ceil(total_size);
  if (order < MIN_ORDER) {
    order = MIN_ORDER;
  }
  if (order > max_order) {
    return NULL;
  }

  uint32_t alloc_order = order;
  while (alloc_order <= max_order && free_lists[alloc_order] == NULL) {
    alloc_order++;
  }

  if (alloc_order > max_order) {
    return NULL;
  }

  void *block = free_lists[alloc_order];
  remove_from_free_list(block, alloc_order);

  while (alloc_order > order) {
    alloc_order--;
    split_block(block, alloc_order + 1U);
    remove_from_free_list(block, alloc_order);
  }

  alloc_header_t *header = (alloc_header_t *)block;
  header->order = order;
  header->magic = ALLOC_MAGIC;

  mem_info.used += (1ULL << order);
  if (mem_info.used > mem_info.size) {
    mem_info.used = mem_info.size;
  }
  mem_info.free = mem_info.size - mem_info.used;

  return (void *)((uint8_t *)block + HEADER_SIZE);
}

void memory_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  alloc_header_t *header = (alloc_header_t *)((uint8_t *)ptr - HEADER_SIZE);
  if (header->magic != ALLOC_MAGIC) {
    return;
  }

  void *block = (void *)header;
  uint32_t order = header->order;

  header->magic = 0;

  if ((1ULL << order) <= mem_info.used) {
    mem_info.used -= (1ULL << order);
  } else {
    mem_info.used = 0;
  }
  mem_info.free = mem_info.size - mem_info.used;

  while (order < max_order) {
    void *buddy = find_and_remove_buddy(block, order);
    if (buddy == NULL) {
      break;
    }

    if (buddy < block) {
      block = buddy;
    }
    order++;
  }

  add_to_free_list(block, order);
}

memory_info_t *memory_get_info() {
  mem_info.free = mem_info.size - mem_info.used;
  return &mem_info;
}

#endif // USE_BUDDY
