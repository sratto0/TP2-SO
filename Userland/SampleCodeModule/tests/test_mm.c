// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "syscalls.h"
#include "test_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;

uint64_t test_mm(uint64_t argc, char *argv[]) {

  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  uint64_t max_memory;

  if (argc != 2)
    return -1;

  if ((max_memory = satoi(argv[1])) <= 0)
    return -1;

  while (1) {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      mm_rqs[rq].address = my_malloc(mm_rqs[rq].size);

      if (mm_rqs[rq].address) {
        printf(" block of size %d\n", mm_rqs[rq].size);
        total += mm_rqs[rq].size;
        rq++;
      }
    }

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address) {
        printf("Setting block at %d, size=%d\n", mm_rqs[i].address,
               mm_rqs[i].size);
        my_memset(mm_rqs[i].address, i, mm_rqs[i].size);
      }

    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address) {
        printf("Checking block at %d, size=%d\n", mm_rqs[i].address,
               mm_rqs[i].size);
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          printf("test_mm ERROR\n");
          return -1;
        }
      }

    // Free
    for (i = 0; i < rq; i++) {
      printf("Freeing block of size %d\n", mm_rqs[i].size);
      if (mm_rqs[i].address) {
        printf("Freeing block of size %d\n", mm_rqs[i].size);
        my_free(mm_rqs[i].address);
      }
    }
  }
  printf("test_mm OK\n");
}