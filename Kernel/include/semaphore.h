#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

#define MAX_SEMAPHORES 64

typedef int lock_t;

typedef struct semaphoreManagerCDT *semaphoreManagerADT;

void semaphore_system_init();

int64_t my_sem_open(char *name, uint64_t initialValue);
int64_t my_sem_wait(char *name);
int64_t my_sem_post(char *name);
int64_t my_sem_close(char *name);

#endif // SEMAPHORE_H