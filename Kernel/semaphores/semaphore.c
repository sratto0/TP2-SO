#include <stddef.h>
#include "../include/semaphore.h"
#include "doubleLinkedList.h"
#include "../include/lib.h"
#include "../include/memoryManager.h"
#include "../include/process.h" 
#include "../include/scheduler.h"


typedef struct semaphore {
    int value;
    int in_use;
    int using_count;
    lock_t lock;
    char name[32];
    DListADT waiting_queue;
}semaphore_t;

typedef struct semaphoreManagerCDT {
    semaphore_t semaphores[MAX_SEMAPHORES];
}semaphoreManagerCDT;

semaphoreManagerADT semaphore_manager = NULL;

extern void acquire_lock(lock_t * lock);
extern void release_lock(lock_t * lock);


static void free_allocated_queues(int last_index) {
    if (semaphore_manager == NULL) {
        return;
    }
    for (int i = 0; i <= last_index; i++) {
        if (semaphore_manager->semaphores[i].waiting_queue != NULL) {
            free_list(semaphore_manager->semaphores[i].waiting_queue);
            semaphore_manager->semaphores[i].waiting_queue = NULL;
        }
    }
}

void semaphore_system_init(){
    if(semaphore_manager != NULL){
        return;
    }

    semaphore_manager = (semaphoreManagerADT) memory_alloc(sizeof(semaphoreManagerCDT));
    if(semaphore_manager == NULL){
        return;
    }
    for(int i = 0; i < MAX_SEMAPHORES; i++){
        semaphore_manager->semaphores[i].value = 0;
        semaphore_manager->semaphores[i].in_use = 0;
        semaphore_manager->semaphores[i].lock = 1;
        semaphore_manager->semaphores[i].using_count = 0;
        semaphore_manager->semaphores[i].waiting_queue = create_list();
        if(semaphore_manager->semaphores[i].waiting_queue == NULL){
            free_allocated_queues(i);
            memory_free(semaphore_manager);
            semaphore_manager = NULL;
            return;
        }
        semaphore_manager->semaphores[i].name[0] = '\0';
    }
}

static semaphore_t * get_semaphore_by_name(char * name){
    for(int i = 0; i < MAX_SEMAPHORES; i++){
        if(semaphore_manager->semaphores[i].in_use && (str_compare(semaphore_manager->semaphores[i].name, name) == 0)){
            return &semaphore_manager->semaphores[i];
        }
    }
    return NULL;
}

static int get_free_slot(){
    for(int i = 0; i < MAX_SEMAPHORES; i++){
        if(!semaphore_manager->semaphores[i].in_use){
            return i;
        }
    }
    return -1;
}

int64_t my_sem_open(char * name, uint64_t initialValue){
    if(semaphore_manager == NULL || name == NULL){
        return -1;
    }

    semaphore_t * sem = get_semaphore_by_name(name);

    if(sem != NULL){
        acquire_lock(&sem->lock);
        sem->using_count++;
        release_lock(&sem->lock);
        return 0;
    }

    int free_index = get_free_slot();
    if(free_index == -1){
        return -1;
    }
    sem = &semaphore_manager->semaphores[free_index];
    sem->value = initialValue;
    sem->in_use = 1;
    sem->lock = 1;
    sem->using_count = 1;
    my_strncpy(sem->name, name, 32);
    sem->name[sizeof(sem->name) - 1] = '\0';
    return 0;
}

int64_t my_sem_wait(char * name){ // Mirar bien donde hacer el release del lock
    if(semaphore_manager == NULL || name == NULL){
        return -1;
    }

    semaphore_t * sem = get_semaphore_by_name(name);
    if(sem == NULL){
        return -1;
    }

    acquire_lock(&sem->lock);

    if(sem->value > 0){
        sem->value--;
        release_lock(&sem->lock);
        return 0;
    }

    int64_t current_pid = get_current_pid();
    if(current_pid == -1){
        release_lock(&sem->lock);
        return -1;
    }

    add_last(sem->waiting_queue, (void *)(uintptr_t)current_pid);

    release_lock(&sem->lock);

    block_process(current_pid);

    return 0;
}

int64_t my_sem_post(char * name){ // Mirar bien donde hacer el release del lock
    if(semaphore_manager == NULL || name == NULL){
        return -1;
    }

    semaphore_t * sem = get_semaphore_by_name(name);
    if(sem == NULL){
        return -1;
    }

    acquire_lock(&sem->lock);

    if(!is_empty(sem->waiting_queue)){
        int64_t pid = (int64_t)(uintptr_t)get_first(sem->waiting_queue);
        delete_first(sem->waiting_queue);
        unblock_process(pid);
    } else {
        sem->value++;
    }

    release_lock(&sem->lock);
    return 0;
}

static void clear_queue(DListADT queue){
    while(!is_empty(queue)){
        delete_first(queue);
    }
}

int64_t my_sem_close(char * name){
    if(semaphore_manager == NULL || name == NULL){
        return -1;
    }

    semaphore_t * sem = get_semaphore_by_name(name);
    if(sem == NULL){
        return -1;
    }

    acquire_lock(&sem->lock);

    if(sem->using_count > 0){
        sem->using_count--;
    }

    if(sem->using_count == 0){
        sem->in_use = 0;
        clear_queue(sem->waiting_queue);
    }
    release_lock(&sem->lock);
    return 0;
}