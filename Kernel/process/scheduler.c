// #include "scheduler.h"
// #include <string.h>

// typedef struct scheduler_t {
//     process_t *processes[MAX_PROCESSES];
//     int process_count;
//     int current_index;
// } scheduler_t;

// static scheduler_t scheduler;

// void scheduler_init() {
//     memset(&scheduler, 0, sizeof(scheduler_t));
// }

// process_t *create_process(const char *name, int priority, int foreground, int parent_pid) {
//     if (scheduler.process_count >= MAX_PROCESSES) return NULL;
//     process_t *p = malloc(sizeof(process_t));
//     p->pid = scheduler.process_count + 1;
//     strncpy(p->name, name, 31);
//     p->priority = priority;
//     p->foreground = foreground;
//     p->state = READY;
//     p->parent_pid = parent_pid;
//     // Inicializar stack, base_pointer, etc.
//     scheduler.processes[scheduler.process_count++] = p;
//     return p;
// }

// // Round Robin con prioridades
// process_t *next_process() {
//     int start = scheduler.current_index;
//     int highest_priority = -1;
//     int next_idx = -1;
//     for (int i = 0; i < scheduler.process_count; i++) {
//         int idx = (start + i) % scheduler.process_count;
//         process_t *p = scheduler.processes[idx];
//         if (p->state == READY && p->priority > highest_priority) {
//             highest_priority = p->priority;
//             next_idx = idx;
//         }
//     }
//     if (next_idx != -1) {
//         scheduler.current_index = next_idx;
//         return scheduler.processes[next_idx];
//     }
//     return NULL;
// }

#include "scheduler.h"
#include <stdio.h>
#include <string.h>

static process_t *queues[PRIORITY_LEVELS][MAX_PROCESSES];
static int q_count[PRIORITY_LEVELS];
static int q_head[PRIORITY_LEVELS];
static process_t *current_proc = NULL;

static void *dummy_sp = NULL;

void scheduler_init(void) {
  for (int i = 0; i < PRIORITY_LEVELS; i++) {
    q_count[i] = 0;
    q_head[i] = 0;
    for (int j = 0; j < MAX_PROCESSES; j++) queues[i][j] = NULL;
  }
  current_proc = NULL;
  dummy_sp = NULL;
}

static void enqueue_at_priority(process_t *p, uint8_t prio) {
  if (!p) return;
  if (prio >= PRIORITY_LEVELS) prio = PRIORITY_LEVELS - 1;
  if (q_count[prio] >= MAX_PROCESSES) return;
  queues[prio][q_count[prio]++] = p;
}

static void remove_from_priority(process_t *p, uint8_t prio) {
  if (!p || prio >= PRIORITY_LEVELS) return;
  for (int i = 0; i < q_count[prio]; i++) {
    if (queues[prio][i] == p) {
      for (int j = i; j < q_count[prio] - 1; j++) queues[prio][j] = queues[prio][j + 1];
      queues[prio][q_count[prio] - 1] = NULL;
      q_count[prio]--;
      if (q_head[prio] >= q_count[prio]) q_head[prio] = 0;
      return;
    }
  }
}

void scheduler_add(process_t *p) {
  if (!p) return;
  if (p->state == PROC_KILLED) return;
  p->state = PROC_READY;
  enqueue_at_priority(p, p->priority);
}

void scheduler_remove(process_t *p) {
  if (!p) return;
  remove_from_priority(p, p->priority);
}

void my_change_priority(process_t *p, uint8_t new_prio) {
  if (!p) return;
  remove_from_priority(p, p->priority);
  p->priority = (new_prio >= PRIORITY_LEVELS) ? (PRIORITY_LEVELS - 1) : new_prio;
  if (p->state == PROC_READY) enqueue_at_priority(p, p->priority);
  
}

process_t *scheduler_next(void) {
  for (int pr = PRIORITY_LEVELS - 1; pr >= 0; pr--) {
    if (q_count[pr] == 0) continue;
    int start = q_head[pr];
    for (int scanned = 0; scanned < q_count[pr]; scanned++) {
      int idx = (start + scanned) % q_count[pr];
      process_t *p = queues[pr][idx];
      if (p && p->state == PROC_READY) {
        q_head[pr] = (idx + 1) % (q_count[pr] ? q_count[pr] : 1);
        return p;
      }
    }
  }
  return NULL;
}

process_t *scheduler_current(void) {
  return current_proc;
}

void scheduler_yield(void) {
  process_t *next = scheduler_next();
  process_t *prev = current_proc;

  if (next == NULL) {
    if (prev && prev->state == PROC_RUNNING) prev->state = PROC_READY;
    current_proc = NULL;
    return;
  }

  if (prev == next) return;

  if (prev && prev->state == PROC_RUNNING) prev->state = PROC_READY;
  next->state = PROC_RUNNING;
  current_proc = next;

  /* Pasamos las direcciones de los campos stack_pointer al asm */
  void **from_ptr = prev ? &prev->stack_pointer : &dummy_sp;
  void **to_ptr = &next->stack_pointer;
  // no tenemos implementado un context_switch
  // context_switch(from_ptr, to_ptr);
}

// /* Nota: context_switch está implementada en ASM (context_switch.S) */
// void context_switch(void **from_sp_ptr, void **to_sp_ptr);