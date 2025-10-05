#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

// void scheduler_init();
// process_t *create_process(const char *name, int priority, int foreground, int parent_pid);
// process_t *next_process();

#include "process.h"

/* Scheduler API */
void scheduler_init(void);
void scheduler_add(process_t *p);
void scheduler_remove(process_t *p);
void scheduler_change_priority(process_t *p, uint8_t new_prio);
process_t *scheduler_next(void);
process_t *scheduler_current(void);
void scheduler_yield(void);

/* Context switch hook: implementar según arquitectura (asm/ISR) */
void context_switch(process_t *from, process_t *to);

#endif