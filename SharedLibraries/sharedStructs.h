#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

#define NO_PID ((int64_t)-1)

#define STDIN 0
#define STDOUT 1
#define STDERR 2


typedef int (*entry_point_t)(int argc, char **argv);

typedef enum {
  PROC_READY,
  PROC_RUNNING,
  PROC_BLOCKED,
  PROC_KILLED
} process_state_t;

typedef struct {
  char name[32];
  int64_t pid;
  int64_t parent_pid;
  uint8_t priority;
  void *stack_base;
  void *stack_pointer;
  void *rip;
  uint8_t foreground;
  process_state_t state;
  uint64_t ticks;
} process_info_t;

typedef struct {
  uint64_t size;
  uint64_t used;
  uint64_t free;
} memory_info_t;

#endif // SHARED_STRUCTS_H
