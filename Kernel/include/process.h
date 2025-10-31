#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "memoryManager.h"
// #include "scheduler.h"
#include "../../SharedLibraries/sharedStructs.h"

#define MAX_NAME_LEN 32
#define DEFAULT_PRIORITY 1

#define INIT_PID 0


typedef struct process {

  int64_t pid;
  int64_t parent_pid;
  char name[MAX_NAME_LEN];
  
  process_state_t state;
  uint8_t priority;
  uint8_t remaining_quantum; 
  uint8_t in_ready_queue;  
  
  void *stack_base;    /* inicio del buffer de stack */
  void *stack_pointer; /* valor actual del SP guardado en el descriptor */
  
  void *stack_top;     /* top (end) del buffer de stack */
  
  entry_point_t entry_point;
  int argc;
  char ** argv;
  
  uint64_t ticks;
  int32_t return_value;
  int64_t waiting_pid; // PID que este proceso esta esperando (o NO_PID)
  
} process_t;

/* Inicialización del subsistema de procesos */
void process_system_init(void);

process_t * my_create_process(int64_t pid, entry_point_t entry_point, char ** argv, char * name, int * fds);
void process_destroy(process_t * proc);
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
