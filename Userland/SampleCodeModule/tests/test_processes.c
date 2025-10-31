#include <stdio.h>
#include "syscall.h"
#include "test_util.h"
#include "stdlib.h"
#include <stddef.h>
#include "../../../SharedLibraries/sharedStructs.h"

typedef struct P_rq {
  int64_t pid;
  process_state_t state;
} p_rq;

int64_t test_processes(uint64_t argc, char *argv[]) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {0};

  if (argc != 1)
    return -1;

  if ((max_processes = satoi(argv[0])) <= 0)
    return -1;

  p_rq p_rqs[max_processes];

  while (1) {

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
      p_rqs[rq].pid = my_create_process((uint64_t)endless_loop, argvAux, "endless_loop", NULL);

      if (p_rqs[rq].pid == -1) {
        printf("test_processes: ERROR creating process\n");
        return -1;
      } else {
        p_rqs[rq].state = PROC_RUNNING;
        alive++;
      }
      printf("Created process with PID %d\n", p_rqs[rq].pid);
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[rq].state == PROC_RUNNING || p_rqs[rq].state == PROC_BLOCKED) {
              if (my_kill(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR killing process\n");
                return -1;
              }
              p_rqs[rq].state = PROC_KILLED;
              alive--;
              printf("Killed process with PID %d\n", p_rqs[rq].pid);
            }
            break;

          case 1:
            if (p_rqs[rq].state == PROC_RUNNING) {
              if (my_block_process(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR blocking process\n");
                return -1;
              }
              p_rqs[rq].state = PROC_BLOCKED;
              printf("Blocked process with PID %d\n", p_rqs[rq].pid);
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == PROC_BLOCKED && GetUniform(100) % 2) {
          if (my_unblock_process(p_rqs[rq].pid) == -1) {
            printf("test_processes: ERROR unblocking process\n");
            return -1;
          }
          p_rqs[rq].state = PROC_RUNNING;
          printf("Unblocked process with PID %d\n", p_rqs[rq].pid);
        }
    }
  }

  printf("Finished");
}
