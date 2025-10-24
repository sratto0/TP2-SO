#include <stdio.h>
#include <string.h>
#include "process.h"

static process_t proc_table[MAX_PROCESSES];
static int32_t next_pid = 1;

void process_system_init(void) {
  memset(proc_table, 0, sizeof(proc_table));
  next_pid = 1;
  scheduler_init();
}

#define STACK_SIZE 0x1000

extern void *setup_stack_frame(void (*wrapper)(entry_point_t, char  **), entry_point_t main, void *stack_top, void *argv);

static char ** duplicate_argv(char ** argv);
static int count_from_argv(char ** argv);

process_t *my_create_process(uint16_t pid, uint16_t parent_pid, entry_point_t entry_point, char ** argv, char * name, uint8_t priority) {
  process_t * proc = my_alloc(sizeof(process_t));

  if (p==NULL){
    return NULL;
  } 

  proc->f = pid;
  proc->parent_pid = parent_pid;
  proc->ticks = 0;
  proc->state = PROC_READY;
  proc->priority = priority;
  proc->quantum = 
  
  proc->stack_base = my_alloc(STACK_SIZE);
  if (proc->stack_base == NULL){
    my_free(proc);
    return NULL;
  }

  proc->stack_pointer = proc->stack_base + STACK_SIZE;
  
  proc_argv = duplicate_argv(argv);
  if (proc->arg == NULL){
    my_free(proc->stack_base);
    my_free(proc);
    return NULL;
  }

  strncpy(proc->name, name, sizeof(proc->name)-1);
  proc->name[sizeof(proc->name)-1] = 0;
  proc->stack_pointer = setup_stack_frame(&process_caller, entry_point, proc->stack_pointer, (void *) proc->argv);
  return proc;
}

void destroy_process(process_t * proc){
  if (proc==NULL){
    return;
  } 
  free_argv(proc->arg);
  my_free(proc->stack_base);
  my_free(proc);
}

//duplicar argumentos para proceso
static char ** duplicate_argv(char ** argv){
  if(argv == NULL || argv[0] == NULL){
    char ** new_argv = my_alloc(sizeof(char *));
    if (new_argv==NULL)
      return NULL;
    new_argv[0] = NULL;
    return new_argv;
  }
  int count = count_from_argv(argv);
  char ** new_argv = my_alloc((count+1)*sizeof(char *));
  if (new_argv==NULL)
    return NULL;
  for (int i=0; i<count; i++){
    int len = strlen(argv[i]) + 1;
    new_argv[i] = my_alloc(len);
    if (new_argv[i]==NULL){
      return NULL;
    }
    memcpy(new_argv[i], argv[i], len);
  }
  new_argv[count] = NULL;
  return new_argv;
}

static int count_from_argv(char ** argv){
  int count = 0;
  while (argv != NULL || argv[count] != NULL){
    count++;
  }
  return count;
}

void free_argv(char ** argv){
  if (argv==NULL){
    return;
  }
  for (int i=0; argv[i]!=NULL; i++){
    my_free(argv[i]);
  }
  my_free(argv);
}

void process_caller(entry_point_t main, char ** argv){
  int count = count_from_argv(argv);
  int64_t ret = main(count, argv);
  my_exit(ret);
}
