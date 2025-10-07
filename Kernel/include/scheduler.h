#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "process.h"

#define PRIORITY_LEVELS 3 /* 0 = low, 1 = med, 2 = high */
#define MAX_PROCESSES 64


/* Scheduler API */
void scheduler_init(void);
void scheduler_add(process_t *p);
void scheduler_remove(process_t *p);
void my_change_priority(process_t *p, uint8_t new_prio);
process_t *scheduler_next(void);
process_t *scheduler_current(void);
void scheduler_yield(void);

// void context_switch(void **from_sp_ptr, void **to_sp_ptr);

#endif