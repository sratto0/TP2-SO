#ifndef PIPES_H
#define PIPES_H

#include <stdint.h>

#define PIPE_BUFFER_SIZE 1024
#define MAX_PIPES 64
#define BUILTIN_FDS 3
#define PIPE_EOF -1
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1
#define SEM_NAME_LEN 32


typedef struct pipeManagerCDT * pipeManagerADT;

void pipe_system_init();
int pipe_create(int fds[2]);
int pipe_write(int fd, const char * buffer, int size);
int pipe_read(int fd, char * buffer, int size);
void send_pipe_eof (int fd);
void pipe_destroy(int fd);


#endif /* PIPES_H */