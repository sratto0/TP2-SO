#include "scheduler.h"
#include "process.h"
#include "memoryManager.h"
#include "doubleLinkedList.h"
#include "time.h"
#include "memoryMap.h"

extern void timer_tick();

static void adopt_children(int64_t pid);
static int remove_process(int64_t pid);
static process_t * get_process(int64_t pid);
static process_t * get_current_process(void);
static void enqueue_ready(process_t * proc);
static process_t * dequeue_ready(void);
static void remove_from_ready_queue(process_t * proc);

static schedulerADT scheduler = NULL;



static void init(int argc, char ** argv) {
  char ** shell_argv = {NULL};

  int shell_pid = add_process((entry_point_t) SHELL_ADDRESS, shell_argv, "shell", NULL);

  set_process_priority(shell_pid, MIN_PRIORITY);

  while(1) {
    _hlt();
  }
}

static int add_init() {
  if (!scheduler || scheduler->process_count != 0) {
    return -1;
  }

  // VER!!: ACA EL NULL QUE PASE COMO FD probablemente cambie
  process_t * pcb_init = my_create_process(INIT_PID, (entry_point_t) init, NULL, "init", NULL);

  if (pcb_init == NULL) {
    return -1;
  }

  pcb_init->priority = MIN_PRIORITY;
  pcb_init->state = PROC_READY;
  pcb_init->remaining_quantum = pcb_init->priority;
  
  scheduler->processes[INIT_PID] = pcb_init;

  scheduler->process_count++;
  
  return 0;
}


void init_scheduler(void){
    if (scheduler != NULL) {
        return;
    }

    scheduler = (schedulerADT) SCHEDULER_ADDRESS;

    for (int i=0; i< MAX_PROCESSES; i++) {
      scheduler->processes[i] = NULL;
    }

      scheduler->current_pid = NO_PID;
      scheduler->process_count = 0;
      scheduler->ready_queue = create_list();
      scheduler->total_cpu_ticks = 0;
      scheduler->force_reschedule = 0;

      
    if (add_init() != 0) {
      memory_free(scheduler);
      scheduler = NULL;
    }
}

schedulerADT get_scheduler() {
  return scheduler;
}


void * schedule(void * prev_rsp) {
    if (scheduler == NULL || scheduler->process_count == 0) {
      scheduler->force_reschedule = 0;
      return prev_rsp;
    }

    process_t * current_process = get_current_process();

    if (current_process == NULL) {
      // VER!!: capaz habria que hacer scheduler->force_reschedule = 0; 
      return prev_rsp;
    }

    if (current_process != NULL) {
      current_process->stack_pointer = prev_rsp;
      current_process->ticks++;
      scheduler->total_cpu_ticks++;

      if (current_process->state == PROC_RUNNING && current_process->remaining_quantum > 0) {
        current_process->remaining_quantum--;
      }

      if (current_process->state == PROC_RUNNING && current_process->remaining_quantum > 0 && !scheduler->force_reschedule) {
        return prev_rsp;
      }

      if (current_process->state == PROC_RUNNING) {
        current_process->state = PROC_READY;
      }

      if (current_process->state == PROC_READY && current_process->pid != INIT_PID) {
        enqueue_ready(current_process);
        if (!current_process->in_ready_queue) { // Si no se pudo encolar (porque no habia espacio o no existia la lista), sigo corriendo el current_process
          current_process->state = PROC_RUNNING;
          current_process->remaining_quantum = current_process->priority;
          scheduler->force_reschedule = 0;
          return prev_rsp;
        }
      }
    }

    process_t * next_process = dequeue_ready();

    if (next_process == NULL) {
      next_process = scheduler->processes[INIT_PID];
    }

    scheduler->current_pid = next_process->pid;
    next_process->state = PROC_RUNNING;
    next_process->remaining_quantum = next_process->priority;
    scheduler->force_reschedule = 0;
    return next_process->stack_pointer;
}


int64_t add_process(entry_point_t main, char ** argv, char * name, int * file_descriptors){
  if(scheduler == NULL || scheduler->process_count >= MAX_PROCESSES){
    return -1;
  }

  int pid = NO_PID;
  for (int i = 0; i < MAX_PROCESSES; i++){
    if(scheduler->processes[i] == NULL){
      pid = i;
      break;
    }
  }

  if (pid == NO_PID){
    return -1;
  }
  
  process_t * new_process = my_create_process(pid, main, argv, name, file_descriptors);
  
  if (new_process == NULL){
    return NO_PID;
  }

  new_process->priority = DEFAULT_PRIORITY;
  new_process->remaining_quantum = new_process->priority;
  new_process->state = PROC_READY;
  new_process->ticks = 0;

  scheduler->processes[pid] = new_process;
  scheduler->process_count++;

  enqueue_ready(new_process);

  return pid;
}

static process_t * get_process(int64_t pid) {
  if (scheduler == NULL || pid < 0 || pid >= MAX_PROCESSES) {
    return NULL;
  }
  return scheduler->processes[pid];
}

static process_t * get_current_process() {
  if (scheduler == NULL || scheduler->current_pid == NO_PID) {
    return NULL;
  }
  return get_process(scheduler->current_pid);
}


void scheduler_destroy() {
    if (scheduler == NULL) {
        return;
    }

    for (uint64_t i = 0; i < MAX_PROCESSES; i++) {
        if (scheduler->processes[i] != NULL) {
            process_destroy(scheduler->processes[i]);
            scheduler->processes[i] = NULL;
        }
    }

    if (scheduler->ready_queue != NULL) {
        free_list(scheduler->ready_queue);
        scheduler->ready_queue = NULL;
    }

    memory_free(scheduler);
    scheduler = NULL;
}


int64_t get_current_pid() {
  if(scheduler == NULL) {
    return -1;
  }
  return scheduler->current_pid;
}

void yield() {
  scheduler->force_reschedule = 1;
  timer_tick();
}

static void adopt_children(int64_t pid){
  if (scheduler == NULL || pid < 0 || pid >= MAX_PROCESSES){
    return;
  }

  for(uint64_t i = 0; i < MAX_PROCESSES; i++){

    process_t * child = scheduler->processes[i];

    if (child != NULL && child->parent_pid == pid){
      child->parent_pid = INIT_PID; // adoptado por init
    }

  }
}


static int remove_process(int64_t pid) {

  if(scheduler == NULL || pid < 0 || pid >= MAX_PROCESSES){ 
    return -1;
  }

  process_t * proc = scheduler->processes[pid];

  if(proc == NULL){
    return -1;
  }

  if (proc->state == PROC_READY || proc->state == PROC_RUNNING) {
    remove_from_ready_queue(proc);
  }

  scheduler->processes[pid] = NULL;
  scheduler->process_count--;
  process_destroy(proc);

  if (scheduler->current_pid == pid) {
    scheduler->current_pid = NO_PID;
    yield();
  }

  return 0;

}


int kill_process(int64_t pid) {
 
  if (get_process(pid) == NULL) {
    return -1;
  }

  adopt_children(pid);

  process_t * proc = scheduler->processes[pid];
  process_t * parent = scheduler->processes[proc->parent_pid];

  if(proc->parent_pid == INIT_PID) {
    remove_process(pid);
  } 
  else if (proc->in_ready_queue) {
      remove_from_ready_queue(proc);
  }
 
  if (proc != NULL && parent != NULL && parent->state == PROC_BLOCKED && parent->waiting_pid == pid) {
    unblock_process(parent->pid);
  }

  if (pid == scheduler->current_pid) {
    yield();
  }

  return 0;
}

int kill_current_process(){
  int64_t pid = get_current_pid();
  if (pid == NO_PID) {
    return -1;
  }
  return kill_process(pid);
}


int block_process(int64_t pid) {
  if (get_process(pid) == NULL) {
    return -1;
  }

  process_t * proc = scheduler->processes[pid];

  if(proc->state != PROC_READY && proc->state != PROC_RUNNING){
    return -1;
  }

  remove_from_ready_queue(proc);
  proc->state = PROC_BLOCKED;

  if (pid == scheduler->current_pid) {
    yield();
  }
  
  return 0;
}

int unblock_process(int64_t pid) {
  process_t * proc = get_process(pid);

  if (proc == NULL || proc->state != PROC_BLOCKED) {
    return -1;
  }
  
  proc->state = PROC_READY;
  proc->remaining_quantum = proc->priority;

  // VER!!: ver si tendriamos que verificar si la lista de readys no es null
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
  if (scheduler == NULL || pid >= MAX_PROCESSES || scheduler->processes[pid] == NULL || priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
    return -1;
  }

  scheduler->processes[pid]->priority = priority;
  scheduler->processes[pid]->remaining_quantum = priority;

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

  if (child->state != PROC_KILLED){
    
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

// int sleep_block(int64_t pid, uint8_t sleep){
//   process_t * proc = get_process(pid);
//   if(pid == 0 || proc == NULL || proc->state == PROC_KILLED){
//     return -1;
//   }
//   if(sleep == 0){
//     remove_sleeping_process(pid);
//   }

//   proc->state = PROC_BLOCKED;
//   proc->quantum = 0;
//   remove_from_ready_queue(proc);
//   add_blocked(proc);

//   if(scheduler != NULL && scheduler->current == pid){
//     yield();
//   }
//   return 0;
// }

void my_exit(int64_t ret) {
  if(scheduler == NULL) {
    return;
  }

  process_t * current = get_current_process();
  adopt_children(current->pid);

  if (scheduler->processes[scheduler->current_pid]->parent_pid == INIT_PID) {
    remove_process(current->pid);
  } 
  else {

    if (current->in_ready_queue || current->state == PROC_RUNNING)  {
      remove_from_ready_queue(current);
    }

    current->state = PROC_KILLED;
    current->return_value = ret;

    process_t * parent = get_process(current->parent_pid);

    if (parent->state == PROC_BLOCKED && parent->waiting_pid == current->pid) {
      unblock_process(parent->pid);
    }

  }

  yield();


}

uint64_t total_ticks(void) {
  if (scheduler == NULL) {
    return 0;
  }
  return scheduler->total_cpu_ticks;
}

// uint8_t foreground_process(int64_t pid){
//   if(scheduler == NULL){
//     return 0;
//   }
//   process_t * shell = get_process(SHELL_PID);
//   if(shell == NULL){
//     return 0;
//   }
//   return shell->waiting_pid == pid;
// }

// process_info_t * get_processes_info(){
//   static process_info_t * processes_info = NULL;
//   static uint64_t info_capacity = 0;

//   if(scheduler == NULL){
//     return NULL;
//   }

//   uint64_t needed = scheduler->size + 1;
//   if (needed == 0) {
//     needed = 1;
//   }

//   if (info_capacity < needed) {
//     process_info_t * new_info = memory_alloc(sizeof(process_info_t) * needed);
//     if (new_info == NULL) {
//       return NULL;
//     }
//     if (processes_info != NULL) {
//       memory_free(processes_info);
//     }
//     processes_info = new_info;
//     info_capacity = needed;
//   }

//   uint64_t idx = 0;
//   for(uint64_t i = 0; i < scheduler->capacity; i++){
//     process_t * proc = scheduler->processes[i];
//     if(proc != NULL){
//       process_info_t * proc_info = &processes_info[idx++];
//       proc_info->pid = proc->pid;
//       proc_info->parent_pid = proc->parent_pid;
//       proc_info->ticks = proc->ticks;
//       proc_info->state = proc->state;
//       proc_info->priority = proc->priority;
//       proc_info->stack_pointer = proc->stack_pointer;
//       proc_info->stack_base = proc->stack_base;
//       my_strncpy(proc_info->name, proc->name, PROCESS_NAME_LEN);
//       proc_info->foreground = foreground_process(proc->pid);
//     }
//   }

//   for(uint64_t j = idx; j < info_capacity; j++) {
//     processes_info[j].pid = NO_PID;
//     processes_info[j].parent_pid = NO_PID;
//     processes_info[j].priority = 0;
//     processes_info[j].stack_pointer = NULL;
//     processes_info[j].stack_base = NULL;
//     processes_info[j].foreground = 0;
//     processes_info[j].state = PROC_KILLED;
//     processes_info[j].ticks = 0;
//     processes_info[j].name[0] = '\0';
//   }

//   return processes_info;
// }

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
  if (scheduler == NULL || scheduler->ready_queue == NULL){
    return;
  }
  if (proc == NULL || !proc->in_ready_queue){
    return;
  }
  delete_element(scheduler->ready_queue, proc);
  proc->in_ready_queue = 0;
}
