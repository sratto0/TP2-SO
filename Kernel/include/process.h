#ifndef PROCESS_H
#define PROCESS_H

// #define MAX_PROCESSES 100

// typedef enum { READY, RUNNING, BLOCKED, TERMINATED } process_state_t;

// typedef struct process_t {
//     int pid;
//     char name[32];
//     int priority;
//     void *stack;
//     void *base_pointer;
//     int foreground;
//     process_state_t state;
//     int parent_pid;
//     // Otros campos necesarios
// } process_t;

#include <stdint.h>
#include "memoryManager.h"
#include "scheduler.h"


typedef int (*entry_point_t)(int argc, char **argv);

typedef struct process {
  uint16_t pid;
  uint16_t parent_pid;

  uint16_t ticks;
  
  proc_state_t state;
  uint8_t priority;
  uint8_t quantum; // remaining quantum
  
 
  void *stack_pointer; /* valor actual del SP guardado en el descriptor */
  void *stack_base;    /* inicio del buffer de stack */
  void *stack_top;     /* top (end) del buffer de stack */
  
  entry_point_t entry_point;
  char ** argv;
  int argc;

  char name[32];

  int32 return_value;
  


} process_t;

/* Inicialización del subsistema de procesos */
void process_system_init(void);

process_t *my_create_process(uint16_t pid, uint16_t parent_pid, entry_point_t entry_point, char ** argv, char * name, uint8_t priority);
void destroy_process(process_t * proc);
void free_argv(char ** argv);
void process_caller(entry_point_t main, char ** argv);

/* Syscall-like API (usados por los tests) */
int32_t my_create_process(const char *name, int priority, char *argv[]);
int my_kill(uint16_t pid);
int my_block(uint16_t pid);
int my_unblock(uint16_t pid);
int my_nice(uint16_t pid, int new_prio);
int32_t my_getpid(void);
void my_list_processes(void);
int my_process_info(uint16_t pid, process_t *out);


#endif