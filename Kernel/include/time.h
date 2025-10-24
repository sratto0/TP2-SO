#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>
#include "scheduler.h"

typedef struct {
	uint32_t pid;
	uint64_t wake_up_tick;
} sleeping_process_t;

/**
 * @brief Handler del timer tick
 */
void timerHandler(uint64_t rsp);
/**
 * @return Devuelve los ticks 
 */
uint64_t ticksElapsed(void);

/**
 * @return  Devuelve los segundos
 */
int secondsElapsed(void);
void init_sleeping_processes(void);
void remove_sleeping_process(uint32_t pid);
void sleep(uint32_t sleeping_ticks);

#endif
