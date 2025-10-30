#ifndef SCHEDULER_H
#define SCHEDULER_H
#define SHELL_PID 1

#include "process.h"
#include "../../SharedLibraries/sharedStructs.h"
#include "lib.h"
#include "doubleLinkedList.h"

#define INITIAL_PROCESS_CAPACITY 32

#define MAX_PROCESSES
// el arreglo de process_t hacerlo estatico
// el chequeo de q no este lleno el array de procesor lo haces con el get_pid, si te devuelve -1 no lo creas



typedef struct schedulerCDT {
  process_t * processes[MAX_PROCESSES];
  int64_t current;
  uint64_t size;
  uint64_t capacity;
  DListADT ready_queue;
  DListADT blocked_queue; // 
  uint64_t total_ticks;
} schedulerCDT;

typedef struct schedulerCDT * schedulerADT; 

void scheduler_init(void);
schedulerADT get_scheduler();
uint64_t total_ticks(void);
void * schedule(void * context);
int64_t add_process(entry_point_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors);
void destroy_scheduler();
int64_t get_current_pid();
void yield();
int kill_process(int64_t pid);
int32_t kill_current_process();
int block_process(int64_t pid);
int unblock_process(int64_t pid);
int block_current_process();
int unblock_current_process();
int set_process_priority(int64_t pid, uint8_t priority);
int64_t wait_pid(int64_t pid, int32_t * exit_code);
int sleep_block(int64_t pid, uint8_t sleep);
void my_exit(int64_t ret);
uint8_t foreground_process(int64_t pid);
process_info_t * get_processes_info();

#endif // SCHEDULER_H
