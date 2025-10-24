#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

#define NO_PID -1 


typedef enum { PROC_READY, PROC_RUNNING, PROC_BLOCKED, PROC_KILLED } process_state_t;

typedef struct {
    charn name[32];
    int16_t pid;
    int16_t p_pid;
    uint8_t priority;
    void * stack_base;
    void stack_pointer;
    uint8_t foreground; 
    process_state_t state;
    uint64_t ticks;
} process_info_t;

#endif // SHARED_STRUCTS_H