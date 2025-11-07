// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "../../../SharedLibraries/sharedStructs.h"
#include "stdlib.h"
#include "syscall.h"
#include "test_util.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc) {
  uint64_t aux = *p;
  my_yield(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]) {
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 4) {
    return -1;
  }

  if ((n = satoi(argv[1])) <= 0)
    return -1;
  if ((inc = satoi(argv[2])) == 0)
    return -1;
  if ((use_sem = satoi(argv[3])) < 0)
    return -1;

  if (use_sem)
    if (my_sem_open(SEM_ID, 1) == -1) {
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++) {
    if (use_sem) {
      my_sem_wait(SEM_ID);
    }
    slowInc(&global, inc);
    if (use_sem) {
      my_sem_post(SEM_ID);
    }
  }

  if (use_sem) {
    my_sem_close(SEM_ID);
  }

  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]) { //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];
  int exit_code;

  if (argc != 3)
    return -1;

  char *argvDec[] = {"my_process_dec", argv[1], "-1", argv[2], NULL};
  char *argvInc[] = {"my_process_inc", argv[1], "1", argv[2], NULL};

  global = 0;

  uint64_t i;
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    int fds[2] = {STDIN, STDOUT};
    pids[i] = my_create_process((entry_point_t)my_process_inc, argvDec,
                                "my_process_inc", fds);
    pids[i + TOTAL_PAIR_PROCESSES] = my_create_process(
        (entry_point_t)my_process_inc, argvInc, "my_process_dec", fds);
  }

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    my_wait_pid(pids[i], &exit_code);
    my_wait_pid(pids[i + TOTAL_PAIR_PROCESSES], &exit_code);
  }

  printf("Final value: %d\n", global);

  return 0;
}