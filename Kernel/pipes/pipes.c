// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "pipes.h"
#include "../include/semaphore.h"
#include <stdlib.h>
#include "../include/memoryManager.h"
#include "../include/lib.h"

typedef struct pipe {
    int in_use;
    int fds[2]; 

    char buffer[PIPE_BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int data_size;

    char space_sem[SEM_NAME_LEN];
    char data_sem[SEM_NAME_LEN];
    char mutex_sem[SEM_NAME_LEN];
} pipe_t;

typedef struct pipeManagerCDT {
    pipe_t pipes[MAX_PIPES];
    int next_fd;
} pipeManagerCDT;


pipeManagerADT pipe_manager = NULL;

void pipe_system_init() {
    if(pipe_manager != NULL){
        return;
    }
    pipe_manager = (pipeManagerADT) memory_alloc(sizeof(pipeManagerCDT));
    if(pipe_manager == NULL){
        return;
    }
    for(int i = 0; i < MAX_PIPES; i++){
        pipe_manager->pipes[i].in_use = 0;
        pipe_manager->pipes[i].fds[0] = -1;
        pipe_manager->pipes[i].fds[1] = -1;
        pipe_manager->pipes[i].read_pos = 0;
        pipe_manager->pipes[i].write_pos = 0;
        pipe_manager->pipes[i].data_size = 0;
        pipe_manager->pipes[i].space_sem[0] = '\0';
        pipe_manager->pipes[i].data_sem[0] = '\0';
        pipe_manager->pipes[i].mutex_sem[0] = '\0';
    }
    pipe_manager->next_fd = BUILTIN_FDS;
}

static void pipe_build_name(int id, const char *suffix, char *dst) {
    char num_buf[12];
    int num_len = itoa((uint64_t)id, num_buf, 10);

    int pos = 0;
    const char *prefix = "pipe_";
    while (prefix[pos] != '\0') {
        dst[pos] = prefix[pos];
        pos++;
    }
    for (int i = 0; i < num_len; i++) {
        dst[pos++] = num_buf[i];
    }
    for (int i = 0; suffix[i] != '\0'; i++) {
        dst[pos++] = suffix[i];
    }
    dst[pos] = '\0';
}

int pipe_create(int fds[2]){
    if(pipe_manager == NULL || fds == NULL){
        return -1;
    }
    for(int i = 0; i < MAX_PIPES; i++){
        if(!pipe_manager->pipes[i].in_use){
            pipe_manager->pipes[i].in_use = 1;
            pipe_manager->pipes[i].fds[0] = pipe_manager->next_fd++;
            pipe_manager->pipes[i].fds[1] = pipe_manager->next_fd++;
            pipe_manager->pipes[i].read_pos = 0;
            pipe_manager->pipes[i].write_pos = 0;
            pipe_manager->pipes[i].data_size = 0;

            pipe_build_name(i, "_space", pipe_manager->pipes[i].space_sem);
            pipe_build_name(i, "_data", pipe_manager->pipes[i].data_sem);
            pipe_build_name(i, "_mutex", pipe_manager->pipes[i].mutex_sem);

            my_sem_open(pipe_manager->pipes[i].space_sem, PIPE_BUFFER_SIZE);
            my_sem_open(pipe_manager->pipes[i].data_sem, 0);
            my_sem_open(pipe_manager->pipes[i].mutex_sem, 1);

            fds[0] = pipe_manager->pipes[i].fds[0];
            fds[1] = pipe_manager->pipes[i].fds[1];
            return 0;
        }
    }

    return -1;
}

static int get_pipe_index_from_fd(int fd, int pos){
    if(pipe_manager == NULL){
        return -1;
    }
    for(int i = 0; i < MAX_PIPES; i++){
        if(pipe_manager->pipes[i].in_use){
            if(pipe_manager->pipes[i].fds[pos] == fd){
                return i;
            }
        }
    }
    return -1;
}

int pipe_write(int fd, const char * buffer, int size){
    if(pipe_manager == NULL || buffer == NULL || size <= 0){
        return -1;
    }
    int index = get_pipe_index_from_fd(fd, PIPE_WRITE_END);
    if(index == -1){
        return -1;
    }
    pipe_t * pipe = &pipe_manager->pipes[index];

    for(int i = 0; i < size; i++){
        my_sem_wait(pipe->space_sem);
        my_sem_wait(pipe->mutex_sem);

        pipe->buffer[pipe->write_pos] = buffer[i];
        pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
        pipe->data_size++;

        my_sem_post(pipe->mutex_sem);
        my_sem_post(pipe->data_sem);
    }
    return size;
}

int pipe_read(int fd, char * buffer, int size){
    if(pipe_manager == NULL || buffer == NULL || size <= 0){
        return -1;
    }

    int index = get_pipe_index_from_fd(fd, PIPE_READ_END);
    if(index == -1){
        return -1;
    }

    pipe_t * pipe = &pipe_manager->pipes[index];

    for(int i = 0; i < size; i++){
        my_sem_wait(pipe->data_sem);
        my_sem_wait(pipe->mutex_sem);

        buffer[i] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUFFER_SIZE;
        pipe->data_size--;

        my_sem_post(pipe->mutex_sem);
        my_sem_post(pipe->space_sem);
    }
    return size;
}

void send_pipe_eof (int fd){
    if(pipe_manager == NULL || fd < BUILTIN_FDS || fd >= pipe_manager->next_fd){
        return;
    }

    int index = get_pipe_index_from_fd(fd, PIPE_WRITE_END);
    if(index == -1){
        return;
    }

    pipe_t * pipe = &pipe_manager->pipes[index];

    my_sem_wait(pipe->space_sem);
    my_sem_wait(pipe->mutex_sem);

    pipe->buffer[pipe->write_pos] = PIPE_EOF;
    pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
    pipe->data_size++;

    my_sem_post(pipe->mutex_sem);
    my_sem_post(pipe->data_sem);

}

void pipe_destroy(int fd){
    if(pipe_manager == NULL || fd < BUILTIN_FDS || fd >= pipe_manager->next_fd){
        return;
    }

    int index = get_pipe_index_from_fd(fd, PIPE_READ_END);
    if(index == -1){
        index = get_pipe_index_from_fd(fd, PIPE_WRITE_END);
        if(index == -1){
            return;
        }
    }

    pipe_t * pipe = &pipe_manager->pipes[index];

    my_sem_close(pipe->space_sem);
    my_sem_close(pipe->data_sem);
    my_sem_close(pipe->mutex_sem);

    pipe->in_use = 0;
    pipe->fds[PIPE_READ_END] = -1;
    pipe->fds[PIPE_WRITE_END] = -1;
    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->data_size = 0;
    pipe->space_sem[0] = '\0';
    pipe->data_sem[0] = '\0';
    pipe->mutex_sem[0] = '\0';
}