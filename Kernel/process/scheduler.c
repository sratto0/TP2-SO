#include "scheduler.h"
#include "process.h"

static void adopt_children(int64_t pid);
static void remove_process(int64_t pid);
static process_t * get_next_process();
static process_t * get_process(int64_t pid);
static process_t * get_current_process(void);
static void reset_quantum(process_t * proc);
static uint8_t priority_to_quantum(uint8_t priority);

static schedulerCDT scheduler_storage;
static schedulerADT scheduler = NULL;
static uint8_t force_reschedule = 0;

void scheduler_init(void){
    if (scheduler != NULL) {
      return;
    }
    
    scheduler = &scheduler_storage;
    for (int i = 0; i < MAX_PROCESSES; i++) {
      scheduler->processes[i] = NULL;
    }
    scheduler->size = 0;
    scheduler->current = -1;
    scheduler->total_ticks = 0;
    force_reschedule = 0;

    //pipe
}

schedulerADT get_scheduler() {
  return scheduler;
}

void * schedule(void * context) {
    if (scheduler == NULL || scheduler->size == 0) {
      force_reschedule = 0;
      return context;
    }

    process_t * current_process = get_current_process();
    if (current_process != NULL) {
        current_process->stack_pointer = context;
        current_process->ticks++;
        scheduler->total_ticks++;
        if (current_process->state == PROC_RUNNING) {
            current_process->state = PROC_READY;
        }
        if (current_process->state == PROC_READY) {
            if (current_process->quantum > 0) {
                current_process->quantum--;
            }
        } else {
            current_process->quantum = 0;
        }
    }
    
    process_t * next = get_next_process();
    if (next == NULL) {
      return context;
    }
    
    scheduler->current = next->pid;
    next->state = PROC_RUNNING;

    return (void *) next->stack_pointer;
}


// pids[i] = my_create_process((uint64_t)zero_to_max, ztm_argv, "zero_to_max", 0, NULL);

int64_t add_process(entry_point_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors){
  if(scheduler == NULL || scheduler->size >= MAX_PROCESSES){
    return -1;
  }

  int64_t pid = NO_PID;
  int64_t found = 0;
  for(int i = 0; i < MAX_PROCESSES; i++){
    if(scheduler->processes[i] == NULL){
      pid = (int64_t)i;
      found = 1;
      break;
    }
  }
  
  if(!found){
    return -1;
  }
  int64_t parent_pid;

  if(scheduler->current == -1) {
    parent_pid = NO_PID;
  } else {
    parent_pid = (int64_t)scheduler->current;
  }
  
  process_t * new_process = my_create_process(pid, parent_pid, main, argv, name, no_kill, file_descriptors, DEFAULT_PRIORITY);
  
  if(new_process == NULL){
    return -1;
  }
  
  //pipe
  reset_quantum(new_process);
  scheduler->processes[pid] = new_process;
  scheduler->size++;
  return pid;
}


static process_t * get_next_process() {
  if (scheduler == NULL || scheduler->size == 0) {
    force_reschedule = 0;
    return NULL;
  }

  process_t * current_process = get_current_process();

  if (!force_reschedule &&
      current_process != NULL &&
      current_process->state == PROC_READY &&
      current_process->quantum > 0) {
    force_reschedule = 0;
    return current_process;
  }

  int64_t start_index = 0;
  if (current_process != NULL) {
    start_index = (scheduler->current + 1) % MAX_PROCESSES;
  }
  int64_t index = start_index;

  do {
    process_t * candidate = scheduler->processes[index];
    if (candidate != NULL && (candidate->state == PROC_READY || candidate->state == PROC_RUNNING)) {
      if (candidate->state == PROC_RUNNING) {
        candidate->state = PROC_READY;
      }
      reset_quantum(candidate);
      force_reschedule = 0;
      return candidate;
    }
    index = (index + 1) % MAX_PROCESSES;
  } while (index != start_index);

  if (current_process != NULL && current_process->state == PROC_READY) {
    if (current_process->quantum == 0) {
      reset_quantum(current_process);
    }
    force_reschedule = 0;
    return current_process;
  }

  force_reschedule = 0;
  return NULL;
}

static process_t * get_process(int64_t pid) {
  if (scheduler == NULL || pid < 0 || pid >= MAX_PROCESSES) {
    return NULL;
  }
  return scheduler->processes[pid];
}

static process_t * get_current_process(void) {
  return get_process(scheduler != NULL ? scheduler->current : NO_PID);
}

static uint8_t priority_to_quantum(uint8_t priority) {
  uint16_t base = (uint16_t)priority + 1;
  if (base > UINT8_MAX) {
    base = UINT8_MAX;
  }
  return (uint8_t)base;
}

static void reset_quantum(process_t * proc) {
  if (proc == NULL) {
    return;
  }
  proc->quantum = priority_to_quantum(proc->priority);
}


void destroy_scheduler(){
  if (scheduler == NULL) {
    return;
  }
  for (int i = 0; i < MAX_PROCESSES; i++){
    if (scheduler->processes[i] != NULL){
      destroy_process(scheduler->processes[i]); //FALTA HACERLO EN EL PROCESS.C
      scheduler->processes[i] = NULL;
    }
  }
  scheduler->size = 0;
  scheduler->current = -1;
  scheduler->total_ticks = 0;
  force_reschedule = 0;
  scheduler = NULL;
}

int64_t get_current_pid() {
  process_t * current = get_current_process();
  if (current == NULL) {
    return NO_PID;
  }
  return current->pid;
}

void yield() {
  force_reschedule = 1;
  //
}

//static void cleanup_process_resources //es de sincro

static void adopt_children(int64_t pid){
  if (scheduler == NULL) {
    return;
  }
  for(int i = 0; i < MAX_PROCESSES; i++)  
    if(scheduler->processes[i] != NULL && scheduler->processes[i]->parent_pid == pid){
      scheduler->processes[i]->parent_pid = 0; //adoptado por init
    }
}

static void remove_process(int64_t pid){
  process_t * proc = get_process(pid);
  if (proc == NULL) {
    return;
  }
  remove_sleeping_process(pid);
  destroy_process(proc);
  scheduler->processes[pid] = NULL;
  if (scheduler->size > 0) {
    scheduler->size--;
  }
  if (scheduler->size == 0 || scheduler->current == pid) {
    scheduler->current = -1;
  }
}


int kill_process(int64_t pid){
  process_t * proc = get_process(pid);
  if (proc == NULL){
    return -1;
  }
  adopt_children(pid);
  process_t * parent = get_process(proc->parent_pid);
  if(parent != NULL && parent->state == PROC_BLOCKED && parent->waiting_pid == pid){
    unblock_process(parent->pid);
  }
  uint8_t context_switch = (scheduler != NULL && scheduler->current == pid);
  remove_sleeping_process(pid);
  proc->state = PROC_KILLED;
  remove_process(pid);

  if(context_switch){
    yield();
  }
  return 0;
}

int32_t kill_current_process(){
  int64_t pid = get_current_pid();
  if (pid == NO_PID) {
    return -1;
  }
  return kill_process(pid);
}

int block_process(int64_t pid){
  process_t * proc = get_process(pid);
  if (proc == NULL){
    return -1;
  }
  
  if(proc->state == PROC_BLOCKED){
    return 0;
  }

  if(proc->state == PROC_KILLED){
    return -1;
  }
  if(proc->state != PROC_READY && proc->state != PROC_RUNNING){
    return -1;
  }
  uint8_t context_switch;
  if(scheduler->current == pid){
    context_switch = 1;
  } else {
    context_switch = 0;
  }

  proc->state = PROC_BLOCKED;
  proc->quantum = 0;

  if(context_switch){
    yield();
  }
  return 0;
}

int unblock_process(int64_t pid){
  process_t * proc = get_process(pid);
  if (proc == NULL){
    return -1;
  }
  if(proc->state != PROC_BLOCKED){
    return -1;
  }

  remove_sleeping_process(pid); 
  proc->state = PROC_READY;
  reset_quantum(proc);

  return 0;
}

int block_current_process(){
  int64_t pid = get_current_pid();
  if (pid == NO_PID) {
    return -1;
  }
  return block_process(pid);
}

int unblock_current_process(){
  int64_t pid = get_current_pid();
  if (pid == NO_PID) {
    return -1;
  }
  return unblock_process(pid);
}

int set_process_priority(int64_t pid, uint8_t priority){
  process_t * proc = get_process(pid);
  if (proc == NULL){
    return -1;
  }
  proc->priority = priority;
  reset_quantum(proc);
  return 0;
}

int64_t wait_pid(int64_t pid, int32_t * exit_code){
  process_t * child = get_process(pid);
  process_t * current = get_current_process();
  if (child == NULL || current == NULL){
    return -1;
  }

  if(child->parent_pid != current->pid){
    return -1;
  }

  if(child->state == PROC_KILLED){
    if(exit_code) {
      *exit_code = child->return_value;
    }
    remove_process(pid);
    current->waiting_pid = NO_PID;
    return pid;
  }

  current->waiting_pid = pid;
  if (block_current_process() != 0) {
    current->waiting_pid = NO_PID;
    return -1;
  }

  child = get_process(pid);
  if(child != NULL && child->state == PROC_KILLED){
    if(exit_code) {
      *exit_code = child->return_value;
    }
    remove_process(pid);
    current->waiting_pid = NO_PID;
    return pid;
  }

  current->waiting_pid = NO_PID;
  return -1;
}

int sleep_block(int64_t pid, uint8_t sleep){
  process_t * proc = get_process(pid);
  if(pid == 0 || proc == NULL || proc->state == PROC_KILLED){
    return -1;
  }
  if(sleep == 0){
    remove_sleeping_process(pid);
  }

  proc->state = PROC_BLOCKED;
  proc->quantum = 0;

  if(scheduler != NULL && scheduler->current == pid){
    yield();
  }
  return 0;
}

void my_exit(int64_t ret){
  process_t * current = get_current_process();
  if(current == NULL) {
    return;
  }
  

  //pipes

  current->state = PROC_KILLED;
  current->return_value = ret;
  current->quantum = 0;
  remove_sleeping_process(current->pid);

  process_t * parent = get_process(current->parent_pid);
  if(parent != NULL && parent->state == PROC_BLOCKED && parent->waiting_pid == current->pid){
    unblock_process(parent->pid);
  }
  yield();
}

uint64_t total_ticks(void) {
  if (scheduler == NULL) {
    return 0;
  }
  return scheduler->total_ticks;
}

uint8_t foreground_process(int64_t pid){
  if(scheduler == NULL){
    return 0;
  }
  process_t * shell = get_process(SHELL_PID);
  if(shell == NULL){
    return 0;
  }
  return shell->waiting_pid == pid;
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

  for(; idx < MAX_PROCESSES; idx++){
    processes_info[idx].pid = NO_PID;
    processes_info[idx].parent_pid = NO_PID;
    processes_info[idx].priority = 0;
    processes_info[idx].stack_pointer = NULL;
    processes_info[idx].stack_base = NULL;
    processes_info[idx].foreground = 0;
    processes_info[idx].state = PROC_KILLED;
    processes_info[idx].ticks = 0;
    processes_info[idx].name[0] = '\0';
  }

  return processes_info;
}
