#include <stdint.h>
#include "process.h"
#include "scheduler.h"
#include "lib.h"
#include "../../SharedLibraries/sharedStructs.h"

void process_system_init(void) {
  init_sleeping_processes();
  scheduler_init();
}

#define STACK_SIZE 0x1000

extern void * set_stack_frame(void (*wrapper)(entry_point_t, char  **), entry_point_t main, void *stack_top, void *argv);

static char ** duplicate_argv(char ** argv);
static int count_from_argv(char ** argv);
static uint32_t str_length(const char * str);
static void free_partial_argv(char ** argv, int allocated);
static uint8_t initial_quantum(uint8_t priority);


process_t * my_create_process(int64_t pid, int64_t parent_pid, entry_point_t entry_point, char ** argv, char * name, uint8_t no_kill, int * fds, uint8_t priority) {
  (void) no_kill;
  (void) fds;
  process_t * proc = memory_alloc(sizeof(process_t));

  if (proc==NULL){
    return NULL;
  } 

  proc->pid = pid;
  proc->parent_pid = parent_pid;
  proc->waiting_pid = NO_PID;
  proc->ticks = 0;
  proc->state = PROC_READY;
  proc->priority = priority;
  proc->quantum = initial_quantum(priority);
  proc->in_ready_queue = 0;
  proc->in_blocked_queue = 0;
  proc->entry_point = entry_point;
  proc->return_value = 0;
  
  proc->stack_base = memory_alloc(STACK_SIZE);
  if (proc->stack_base == NULL){
    memory_free(proc);
    return NULL;
  }

  proc->stack_pointer = (uint8_t *)proc->stack_base + STACK_SIZE;
  proc->stack_top = proc->stack_pointer;
  
  proc->argv = duplicate_argv(argv);
  if (proc->argv == NULL){
    memory_free(proc->stack_base);
    memory_free(proc);
    return NULL;
  }

  proc->argc = count_from_argv(proc->argv);

  if (name != NULL){
    my_strncpy(proc->name, name, sizeof(proc->name));
  } else {
    proc->name[0] = 0;
  }
  proc->stack_pointer = set_stack_frame(&process_caller, entry_point, proc->stack_pointer, proc->argv);
  return proc;
}

void destroy_process(process_t * proc){
  if (proc==NULL){
    return;
  } 
  free_argv(proc->argv);
  memory_free(proc->stack_base);
  memory_free(proc);
}

//duplicar argumentos para proceso
static char ** duplicate_argv(char ** argv){
  if(argv == NULL || argv[0] == NULL){
    char ** new_argv = memory_alloc(sizeof(char *));
    if (new_argv==NULL)
      return NULL;
    new_argv[0] = NULL;
    return new_argv;
  }
  int count = count_from_argv(argv);
  char ** new_argv = memory_alloc((count+1)*sizeof(char *));
  if (new_argv==NULL)
    return NULL;
  for (int i=0; i<count; i++){
    uint32_t len = str_length(argv[i]) + 1;
    new_argv[i] = memory_alloc(len);
    if (new_argv[i]==NULL){
      free_partial_argv(new_argv, i);
      memory_free(new_argv);
      return NULL;
    }
    memcpy(new_argv[i], argv[i], len);
  }
  new_argv[count] = NULL;
  return new_argv;
}

static int count_from_argv(char ** argv){
  int count = 0;
  if (argv == NULL){
    return 0;
  }
  while (argv[count] != NULL){
    count++;
  }
  return count;
}

void free_argv(char ** argv){
  if (argv==NULL){
    return;
  }
  for (int i=0; argv[i]!=NULL; i++){
    memory_free(argv[i]);
  }
  memory_free(argv);
}

void process_caller(entry_point_t main, char ** argv){
  int count = count_from_argv(argv);
  int64_t ret = main(count, argv);
  my_exit(ret);
}

static uint32_t str_length(const char * str){
  if (str == NULL){
    return 0;
  }
  uint32_t len = 0;
  while (str[len] != 0){
    len++;
  }
  return len;
}

static void free_partial_argv(char ** argv, int allocated){
  if (argv == NULL){
    return;
  }
  for (int i = 0; i < allocated; i++){
    if (argv[i] != NULL){
      memory_free(argv[i]);
    }
  }
}

static uint8_t initial_quantum(uint8_t priority){
  uint16_t base = (uint16_t)priority + 1;
  if (base > UINT8_MAX) {
    base = UINT8_MAX;
  }
  return (uint8_t)base;
}
