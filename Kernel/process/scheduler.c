#include "scheduler.h"
#include "process.h"
#include "memoryManager.h"
#include "doubleLinkedList.h"
#include "time.h"

static void adopt_children(int64_t pid);
static void remove_process(int64_t pid);
static process_t * get_process(int64_t pid);
static process_t * get_current_process(void);
static void enqueue_ready(process_t * proc);
static process_t * dequeue_ready(void);
static void remove_from_ready_queue(process_t * proc);
static void add_blocked(process_t * proc);
static void remove_from_blocked(process_t * proc);
static void reset_quantum(process_t * proc);
static uint8_t priority_to_quantum(uint8_t priority);
static int64_t reserve_pid(void);
static int ensure_capacity(uint64_t min_capacity);
static int64_t find_free_slot(void);

static schedulerCDT scheduler_storage;
static schedulerADT scheduler = NULL;
static uint8_t force_reschedule = 0;

void scheduler_init(void){
    if (scheduler != NULL) {
        return;
    }

    scheduler = &scheduler_storage;
    scheduler->processes = NULL;
    scheduler->capacity = 0;
    scheduler->size = 0;
    scheduler->current = NO_PID;
    scheduler->total_ticks = 0;
    scheduler->ready_queue = createDList();
    scheduler->blocked_queue = createDList();
    force_reschedule = 0;

    if (scheduler->ready_queue == NULL || scheduler->blocked_queue == NULL || ensure_capacity(INITIAL_PROCESS_CAPACITY) != 0) {
        if (scheduler->ready_queue != NULL) {
            free_list(scheduler->ready_queue);
            scheduler->ready_queue = NULL;
        }
        if (scheduler->blocked_queue != NULL) {
            free_list(scheduler->blocked_queue);
            scheduler->blocked_queue = NULL;
        }
        scheduler = NULL;
    }
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

        if (current_process->state == PROC_READY &&
            !force_reschedule &&
            current_process->quantum > 0) {
            current_process->quantum--;
            if (current_process->quantum > 0) {
                current_process->state = PROC_RUNNING;
                force_reschedule = 0;
                return context;
            }
        }

        if (current_process->state == PROC_READY) {
            reset_quantum(current_process);
            enqueue_ready(current_process);
        }
    }

    process_t * next = dequeue_ready();
    if (next == NULL) {
        scheduler->current = NO_PID;
        force_reschedule = 0;
        return context;
    }

    scheduler->current = next->pid;
    next->state = PROC_RUNNING;
    force_reschedule = 0;
    return (void *) next->stack_pointer;
}

static int64_t reserve_pid(void) {
    int64_t pid = find_free_slot();
    if (pid != NO_PID) {
        return pid;
    }

    uint64_t old_capacity = scheduler->capacity;
    uint64_t target = (old_capacity == 0) ? INITIAL_PROCESS_CAPACITY : old_capacity * 2;
    if (ensure_capacity(target) != 0) {
        return NO_PID;
    }
    return (int64_t)old_capacity;
}

// pids[i] = my_create_process((uint64_t)zero_to_max, ztm_argv, "zero_to_max", 0, NULL);

int64_t add_process(entry_point_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors){
  if(scheduler == NULL){
    return -1;
  }

  int64_t pid = reserve_pid();
  if (pid == NO_PID) {
    return -1;
  }

  int64_t parent_pid = (scheduler->current == NO_PID) ? NO_PID : scheduler->current;
  
  process_t * new_process = my_create_process(pid, parent_pid, main, argv, name, no_kill, file_descriptors, DEFAULT_PRIORITY);
  
  if(new_process == NULL){
    return -1;
  }
  
  reset_quantum(new_process);
  scheduler->processes[pid] = new_process;
  scheduler->size++;
  enqueue_ready(new_process);
  return pid;
}

static process_t * get_process(int64_t pid) {
  if (scheduler == NULL || scheduler->processes == NULL || pid < 0 || (uint64_t)pid >= scheduler->capacity) {
    return NULL;
  }
  return scheduler->processes[pid];
}

static process_t * get_current_process(void) {
  if (scheduler == NULL || scheduler->current == NO_PID) {
    return NULL;
  }
  return get_process(scheduler->current);
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

  if (scheduler->processes != NULL) {
    for (uint64_t i = 0; i < scheduler->capacity; i++){
      if (scheduler->processes[i] != NULL){
        destroy_process(scheduler->processes[i]);
        scheduler->processes[i] = NULL;
      }
    }
    memory_free(scheduler->processes);
    scheduler->processes = NULL;
  }

  if (scheduler->ready_queue != NULL) {
    free_list(scheduler->ready_queue);
    scheduler->ready_queue = NULL;
  }

  if (scheduler->blocked_queue != NULL) {
    free_list(scheduler->blocked_queue);
    scheduler->blocked_queue = NULL;
  }

  scheduler->capacity = 0;
  scheduler->size = 0;
  scheduler->current = NO_PID;
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
}

static void adopt_children(int64_t pid){
  if (scheduler == NULL || scheduler->processes == NULL) {
    return;
  }
  for(uint64_t i = 0; i < scheduler->capacity; i++){
    process_t * child = scheduler->processes[i];
    if(child != NULL && child->parent_pid == pid){
      child->parent_pid = 0; // adoptado por init
    }
  }
}

static void remove_process(int64_t pid){
  process_t * proc = get_process(pid);
  if (proc == NULL) {
    return;
  }

  remove_from_ready_queue(proc);
  remove_from_blocked(proc);
  remove_sleeping_process(pid);
  destroy_process(proc);
  scheduler->processes[pid] = NULL;

  if (scheduler->size > 0) {
    scheduler->size--;
  }
  if (scheduler->size == 0 || scheduler->current == pid) {
    scheduler->current = NO_PID;
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
  uint8_t context_switch = (scheduler->current == pid);

  proc->state = PROC_BLOCKED;
  proc->quantum = 0;
  remove_from_ready_queue(proc);
  add_blocked(proc);

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
  remove_from_blocked(proc);
  enqueue_ready(proc);

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
  remove_from_ready_queue(proc);
  add_blocked(proc);

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
  
  current->state = PROC_KILLED;
  current->return_value = ret;
  current->quantum = 0;
  remove_from_ready_queue(current);
  remove_from_blocked(current);
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
  static process_info_t * processes_info = NULL;
  static uint64_t info_capacity = 0;

  if(scheduler == NULL){
    return NULL;
  }

  uint64_t needed = scheduler->size + 1;
  if (needed == 0) {
    needed = 1;
  }

  if (info_capacity < needed) {
    process_info_t * new_info = memory_alloc(sizeof(process_info_t) * needed);
    if (new_info == NULL) {
      return NULL;
    }
    if (processes_info != NULL) {
      memory_free(processes_info);
    }
    processes_info = new_info;
    info_capacity = needed;
  }

  uint64_t idx = 0;
  for(uint64_t i = 0; i < scheduler->capacity; i++){
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

  for(uint64_t j = idx; j < info_capacity; j++) {
    processes_info[j].pid = NO_PID;
    processes_info[j].parent_pid = NO_PID;
    processes_info[j].priority = 0;
    processes_info[j].stack_pointer = NULL;
    processes_info[j].stack_base = NULL;
    processes_info[j].foreground = 0;
    processes_info[j].state = PROC_KILLED;
    processes_info[j].ticks = 0;
    processes_info[j].name[0] = '\0';
  }

  return processes_info;
}

static void enqueue_ready(process_t * proc){
  if (scheduler == NULL || scheduler->ready_queue == NULL || proc == NULL){
    return;
  }
  if (proc->in_ready_queue){
    return;
  }
  proc->in_ready_queue = 1;
  proc->state = PROC_READY;
  if (add_last(scheduler->ready_queue, proc) == -1){
    proc->in_ready_queue = 0;
  }
}

static process_t * dequeue_ready(void){
  if (scheduler == NULL || scheduler->ready_queue == NULL){
    return NULL;
  }
  while (!is_empty(scheduler->ready_queue)){
    process_t * proc = get_first(scheduler->ready_queue);
    delete_first(scheduler->ready_queue);
    if (proc != NULL){
      proc->in_ready_queue = 0;
      if (proc->state == PROC_READY){
        return proc;
      }
    }
  }
  return NULL;
}

static void remove_from_ready_queue(process_t * proc){
  if (scheduler == NULL || scheduler->ready_queue == NULL || proc == NULL){
    return;
  }
  if (!proc->in_ready_queue){
    return;
  }
  delete_element(scheduler->ready_queue, proc);
  proc->in_ready_queue = 0;
}

static void add_blocked(process_t * proc){
  if (scheduler == NULL || scheduler->blocked_queue == NULL || proc == NULL){
    return;
  }
  if (proc->in_blocked_queue){
    return;
  }
  proc->in_blocked_queue = 1;
  if (add_last(scheduler->blocked_queue, proc) == -1){
    proc->in_blocked_queue = 0;
  }
}

static void remove_from_blocked(process_t * proc){
  if (scheduler == NULL || scheduler->blocked_queue == NULL || proc == NULL){
    return;
  }
  if (!proc->in_blocked_queue){
    return;
  }
  delete_element(scheduler->blocked_queue, proc);
  proc->in_blocked_queue = 0;
}

static int ensure_capacity(uint64_t min_capacity){
  if (scheduler == NULL){
    return -1;
  }

  if (scheduler->capacity >= min_capacity){
    return 0;
  }

  uint64_t new_capacity = (scheduler->capacity == 0) ? INITIAL_PROCESS_CAPACITY : scheduler->capacity;
  while (new_capacity < min_capacity){
    new_capacity *= 2;
  }

  process_t ** new_table = memory_alloc(sizeof(process_t *) * new_capacity);
  if (new_table == NULL){
    return -1;
  }

  for(uint64_t i = 0; i < new_capacity; i++){
    new_table[i] = NULL;
  }

  if (scheduler->processes != NULL){
    for(uint64_t i = 0; i < scheduler->capacity; i++){
      new_table[i] = scheduler->processes[i];
    }
    memory_free(scheduler->processes);
  }

  scheduler->processes = new_table;
  scheduler->capacity = new_capacity;
  return 0;
}

static int64_t find_free_slot(void){
  if (scheduler == NULL || scheduler->processes == NULL){
    return NO_PID;
  }

  for(uint64_t i = 0; i < scheduler->capacity; i++){
    if (scheduler->processes[i] == NULL){
      return (int64_t)i;
    }
  }
  return NO_PID;
}
