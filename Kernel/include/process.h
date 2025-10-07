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

typedef enum { PROC_READY, PROC_RUNNING, PROC_BLOCKED, PROC_KILLED } proc_state_t;


typedef struct process {
    int32_t pid;
  char name[32];
  uint8_t priority;
  proc_state_t state;
  void *stack_pointer; /* valor actual del SP guardado en el descriptor */
  void *stack_base;    /* inicio del buffer de stack */
  void *stack_top;     /* top (end) del buffer de stack */
  void (*entry)(void *);
  void *arg;
} process_t;

/* Inicialización del subsistema de procesos */
void process_system_init(void);

/* Syscall-like API (usados por los tests) */
int32_t my_create_process(const char *name, int priority, char *argv[]);
int my_kill(int32_t pid);
int my_block(int32_t pid);
int my_unblock(int32_t pid);
int my_nice(int32_t pid, int new_prio);
int32_t my_getpid(void);
void my_list_processes(void);
int my_process_info(int32_t pid, process_t *out);


#endif