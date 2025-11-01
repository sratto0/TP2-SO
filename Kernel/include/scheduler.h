#ifndef SCHEDULER_H
#define SCHEDULER_H
#define SHELL_PID 1

#include "process.h"
#include "../../SharedLibraries/sharedStructs.h"
#include "lib.h"
#include "doubleLinkedList.h"

#define INITIAL_PROCESS_CAPACITY 32
#define MIN_PRIORITY 0
#define MAX_PRIORITY 5
#define PRIORITY_LEVELS (MAX_PRIORITY - MIN_PRIORITY + 1)

#define MAX_PROCESSES 64


typedef struct schedulerCDT {
  process_t * processes[MAX_PROCESSES];   // Array para acceso al PCB por PID
  int64_t current_pid;                    // PID del proceso actual  
  uint64_t process_count;                 // Cantidad total de procesos                            
  DListADT ready_queues[PRIORITY_LEVELS]; // Listas de procesos PROC_READY por prioridad
  uint64_t total_cpu_ticks;               // Total de ticks de la CPU
  uint8_t force_reschedule;               // Flag para forzar el cambio del proces corriendo
} schedulerCDT;

typedef struct schedulerCDT * schedulerADT; 

void init_scheduler(void);
uint64_t total_ticks(void);
void * schedule(void * context);
int64_t add_process(entry_point_t main, char ** argv, char * name, int * file_descriptors);
void destroy_scheduler();
int64_t get_current_pid();
void yield();
int kill_process(int64_t pid);
int kill_current_process();
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
