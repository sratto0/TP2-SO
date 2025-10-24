#include "scheduler.h"
#include "process.h"

static void adopt_children(uint16_t pid);
static void remove_process(uint16_t pid);
static process_t * get_next_process();

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

    scheduler->total_ticks = 0;

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
        scheduler->processes[scheduler->current]->ticks++;
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

int64_t add_process(entry_point_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors){
  if(scheduler == NULL || scheduler->size >= MAX_PROCESSES || file_descriptors == NULL){
    return -1;
  }
  
  uint16_t pid = -1;
  uint16_t found = 0;
  for(int i = 0; i < MAX_PROCESSES; i++){
    if(scheduler->processes[i] == NULL){
      pid = i;
      found = 1;
      break;
    }
  }
  
  if(!found){
    return -1;
  }
  uint16_t parent_pid;

  if(scheduler->current == -1) {
    parent_pid = -1;
  } else {
    parent_pid = scheduler->current;
  }
  
  process_t * new_process = my_create_process(pid, parent_pid, main, argv, name, no_kill, file_descriptors, DEFAULT_PRIORITY);
  
  if(new_process == NULL){
    return -1;
  }
  
  //pipe

  scheduler->processes[pid] = new_process;
  scheduler->size++;
  return pid;
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

uint16_t get_current_pid() {
  if (scheduler == NULL || scheduler->current == -1) {
    return -1;
  }
  return scheduler->current;
}

void yield() {
  force_reschedule = 1;
  //
}

//static void cleanup_process_resources //es de sincro

static void adopt_children(uint16_t pid){
  for(int i = 0; ;)
    if(scheduler->processes[i] != NULL && scheduler->processes[i]->parent_pid == pid){
      scheduler->processes[i]->parent_pid = 0; //adoptado por init
    }
}

static void remove_process(uint16_t pid){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return;
  }
  destroy_process(scheduler->processes[pid]);
  scheduler->processes[pid] = NULL;
  scheduler->size--;
}


int kill_process(uint16_t pid){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return -1;
  }
  adopt_children(pid);
  process_t * proc = scheduler->processes[pid];
  process_t * parent = scheduler->processes[proc->parent_pid];
  if(parent != NULL && parent->state == PROC_BLOCKED){
    //unblock process
  }
  uint8_t context_switch;
  if(scheduler->current == pid){
    context_switch = 1;
  } else {
    context_switch = 0;
  }

  proc->state = PROC_KILLED;
  remove_process(pid);

  if(context_switch){
    yield();
  }
  return 0;
}

int32_t kill_current_process(){
  return kill_process(scheduler->current);
}

int block_process(uint16_t pid){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return -1;
  }
  process_t * proc = scheduler->processes[pid];
  if(proc->state != PROC_RUNNING && proc->state != PROC_READY){
    return -1;
  }
  uint8_t context_switch;
  if(scheduler->current == pid){
    context_switch = 1;
  } else {
    context_switch = 0;
  }

  proc->state = PROC_BLOCKED;

  if(context_switch){
    yield();
  }
  return 0;
}

int unblock_process(uint16_t pid){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return -1;
  }
  process_t * proc = scheduler->processes[pid];
  if(proc->state != PROC_BLOCKED){
    return -1;
  }

  remove_sleeping_process(pid); 
  proc->state = PROC_READY;

  return 0;
}

int block_current_process(){
  return block_process(scheduler->current);
}

int unblock_current_process(){
  return unblock_process(scheduler->current);
}

int set_process_priority(uint16_t pid, uint8_t priority){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return -1;
  }
  process_t * proc = scheduler->processes[pid];
  proc->priority = priority;
  proc->quantum = priority;
  return 0;
}

int64_t wait_pid(uint16_t pid, int32_t * exit_code){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return -1;
  }

  process_t * proc = scheduler->processes[pid];
  if(proc->parent_pid != scheduler->current){
    return -1;
  }

  if(proc->state == PROC_KILLED){
    if(exit_code) *exit_code = proc->return_value;
    remove_process(pid);
    return pid;
  }

  process_t * current = scheduler->processes[scheduler->current];
  current->waiting_pid = pid;
  block_current_process();

  if(scheduler->processes[pid] != NULL){
    proc = scheduler->processes[pid];
    if(proc->state == PROC_KILLED){
      if(exit_code) *exit_code = proc->return_value;
      remove_process(pid);
      return pid;
    }
  }

  return -1;
}

int sleep_block(uint16_t pid, uint8_t sleep){
  if(pid == 0 || scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL || scheduler->processes[pid]->state == PROC_KILLED){
    return -1;
  }
  if(sleep == 0){
    remove_sleeping_process(pid);
  }

  scheduler->processes[pid]->state = PROC_BLOCKED;

  if(pid == scheduler->current){
    yield();
  }
  return 0;
}

void my_exit(int64_t ret){
  if(scheduler == NULL)
  return;
  process_t *current = scheduler->processes[scheduler->current];
  

  //pipes

  current->state = PROC_KILLED;
  current->return_value = ret;
  remove_sleeping_process(current->pid);

  process_t * parent = scheduler->processes[current->parent_pid];
  if(parent != NULL && parent->state == PROC_BLOCKED && parent->waiting_pid == current->pid){
    unblock_process(parent->pid);
  }
  remove_process(current->pid);
  yield();
}

uint16_t total_ticks() {
  if (scheduler == NULL) {
    return 0;
  }
  return scheduler->total_ticks;
}

uint8_t foreground_process(uint16_t pid){
  if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL){
    return 0;
  }
  return scheduler->processes[SHELL_PID]->waiting_pid = pid;
}

process_info_t * get_processes_info(){
  static process_info_t processes_info[MAX_PROCESSES];
  if(scheduler == NULL){
    return NULL;
  }
  int idx = 0;
  for(int i = 0; i < MAX_PROCESSES; i++){
    process_t * proc = scheduler->processes[i];
    if(proc != NULL){
      process_info_t * proc_info = &processes_info[idx++];
      proc_info->pid = proc->pid;
      proc_info->parent_pid = proc->parent_pid;
      proc_info->ticks = proc->ticks;
      proc_info->state = proc->state;
      proc_info->priority = proc->priority;
      proc_info->stack_pointer = proc->stack_pointer;
      proc_info->stack_base = proc->stack_base;
      my_strncpy(proc_info->name, proc->name, PROCESS_NAME_LEN);
      proc_info->foreground = foreground_process(proc->pid);
    }
  }

  //

  return processes_info;
}