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
void timerHandler();
/**
 * @return Devuelve los ticks 
 */
uint64_t ticksElapsed();

/**
 * @return  Devuelve los segundos
 */
int secondsElapsed();

#endif
