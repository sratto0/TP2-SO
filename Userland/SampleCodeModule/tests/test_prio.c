#include <stdint.h>
#include <stdio.h>
#include "syscall.h"
#include "test_util.h"
#include "stdlib.h"
#include <stddef.h>
#include "../../../SharedLibraries/sharedStructs.h"

#define TOTAL_PROCESSES 60
#define PRIOS 3

#define LOWEST 0  // TODO: Change as required
#define MEDIUM 1  // TODO: Change as required
#define HIGHEST 2 // TODO: Change as required

int64_t prio[PRIOS] = {LOWEST, MEDIUM, HIGHEST};

uint64_t max_value = 0;

int zero_to_max(int argc, char *argv[]) {
  uint64_t value = 0;

  while (value++ != max_value);

  printf("PROCESS %d DONE!\n", my_getpid());
  return 0;
}

uint64_t test_prio(uint64_t argc, char *argv[]) {
  int64_t pids[TOTAL_PROCESSES];
  char *ztm_argv[] = {0};
  int64_t i;
  int exit_code;

  if (argc != 1)
    return -1;

  if ((max_value = satoi(argv[0])) <= 0)
    return -1;

  printf("SAME PRIORITY...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = my_create_process((entry_point_t)zero_to_max, ztm_argv, "zero_to_max", NULL);

  // Expect to see them finish at the same time

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait_pid(pids[i], &exit_code);

  printf("SAME PRIORITY, THEN CHANGE IT...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process((entry_point_t)zero_to_max, ztm_argv, "zero_to_max", NULL);
    my_nice(pids[i], prio[i%3]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i%3]);
  }

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait_pid(pids[i], &exit_code);

  printf("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process((entry_point_t)zero_to_max, ztm_argv, "zero_to_max", NULL);
    my_block_process(pids[i]);
    my_nice(pids[i], prio[i%3]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i%3]);
  }

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_unblock_process(pids[i]);

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait_pid(pids[i], &exit_code);

  printf("Finished\n");
  return 0;
}
