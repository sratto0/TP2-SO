#include "scheduler.h"
#include "process.h"

#define MAX_PROCESSES 100


typedef struct schedulerCDT {
  process_t * processes[MAX_PROCESSES + 1];
  int16_t current;
  uint8_t size;
};

typedef struct schedulerCDT * schedulerADT; 

static schedulerADT scheduler = NULL;
uint8_t force_reschedule = 0;

void scheduler_init(){
    if (scheduler != NULL) {
      return;
    }
    
    scheduler = (schedulerADT) SCHEDULER_ADDRESS;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
      scheduler->processes[i] = NULL;
    }
    scheduler->size = 0;
    scheduler->current = -1;
    // total_cpu_ticks ni idea

    //pipe
}

schedulerADT get_scheduler() {
  return scheduler;
}

void * schedule(void * context) {
    if(scheduler == NULL || scheduler->size == 0) {
      return context;
    }

    if(scheduler->current != -1){
        process_t * current_process = scheduler->processes[scheduler->current];
        current_process->stack_pointer = context;
        if(current_process->state == PROC_RUNNING){
            current_process->state = PROC_READY;
        }
    }
    
    process_t * next =  get_next_process();
    if(next == NULL){
        return context;
    }
    
    scheduler->current = next->pid;
    next->state = PROC_RUNNING;

    return (void *) next->stack_pointer;
}


static process_t * get_next_process() {
  if (scheduler == NULL || scheduler->size == 0) {
    force_reschedule = 0;
    return NULL;
  }

  process_t * current_process = scheduler->processes[scheduler->current];
  
  if(current_process != NULL && current_process->state == PROC_RUNNING && current_process->quantum > 0 && !force_reschedule){
    current_process->quantum--;
    return current_process;
  }

  int16_t next_index;
  if(scheduler->current == -1){
    next_index = 0;
  } else {
    next_index = (scheduler->current + 1) % MAX_PROCESSES;
  }
  int16_t start_index = next_index;

    while (1) {
        process_t * candidate = scheduler->processes[next_index];
        if (candidate != NULL && candidate->state == PROC_READY) {
        force_reschedule = 0;
        candidate->quantum = candidate->priority; 
        return candidate;
        }
        next_index = (next_index + 1) % MAX_PROCESSES;
        if (next_index == start_index) {
        break;
        }
    }
    force_reschedule = 0;
    return NULL;
}


void destroy_scheduler(){
  for(int i = 0; i < MAX_PROCESSES; i++){
    if(scheduler->processes[i] != NULL){
      destroy_process(scheduler->processes[i]); //FALTA HACERLO EN EL PROCESS.C
    }
  }
  scheduler = NULL;
}

