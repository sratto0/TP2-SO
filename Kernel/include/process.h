#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "memoryManager.h"
// #include "scheduler.h"
#include "../../SharedLibraries/sharedStructs.h"

#define PROCESS_NAME_LEN 32
#define DEFAULT_PRIORITY 1
typedef int (*entry_point_t)(int argc, char **argv);

typedef struct process {
  int64_t pid;
  int64_t parent_pid;
  int64_t waiting_pid; // PID que este proceso esta esperando (o NO_PID)
  uint64_t ticks;
  
  process_state_t state;
  uint8_t priority;
  uint8_t quantum; // remaining quantum
  
 
  void *stack_pointer; /* valor actual del SP guardado en el descriptor */
  void *stack_base;    /* inicio del buffer de stack */
  void *stack_top;     /* top (end) del buffer de stack */
  
  entry_point_t entry_point;
  char ** argv;
  int argc;

  char name[32];

  int32_t return_value;

} process_t;

/* Inicialización del subsistema de procesos */
void process_system_init(void);

process_t * my_create_process(int64_t pid, int64_t parent_pid, entry_point_t entry_point, char ** argv, char * name, uint8_t no_kill, int * fds, uint8_t priority);
void destroy_process(process_t * proc);
void free_argv(char ** argv);
void process_caller(entry_point_t main, char ** argv);

/* Syscall-like API (usados por los tests) */
// int32_t my_create_process(const char *name, int priority, char *argv[]);
// int my_kill(int64_t pid);
// int my_block(int64_t pid);
// int my_unblock(int64_t pid);
// int my_nice(int64_t pid, int new_prio);
// int32_t my_getpid(void);
// void my_list_processes(void);
// int my_process_info(int64_t pid, process_t *out);


#endif
