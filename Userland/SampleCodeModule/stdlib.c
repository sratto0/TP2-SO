// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "stdlib.h"
#include "sharedStructs.h"
#include <stdint.h>
#include <stdio.h>

#define toLower(n) ((n) >= 'A' && (n) <= 'Z' ? (n) - ('A' - 'a') : (n))
#define isNumber(n) ((n) >= '0' && (n) <= '9')
#define isHex(n) ((n) >= 'a' && (n) <= 'f')

static unsigned int log(uint64_t n, int base) {
  unsigned int count = 1;
  while (n /= base)
    count++;
  return count;
}

int itoa(uint64_t n, char *buffer, int base) {

  if (n == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return 1;
  }

  unsigned int len = 0;
  int i = 0;
  if (n < 0 && base == 10) {
    n = -n;
    buffer[i] = '-';
    len++;
    i++;
  }

  len += log(n, base);
  while (n != 0) {
    int r = n % base;
    buffer[len - i++ - 1] = (r > 9) ? (r - 10) + 'A' : r + '0';
    n /= base;
  }
  buffer[i] = '\0';
  return len;
}

int atoi(char *s) {
  int num = 0;
  while (isNumber(*s))
    num = num * 10 + *(s++) - '0';
  return num;
}

int strtoi(char *s, char **end) {
  int num = 0;
  while (isNumber(*s))
    num = num * 10 + *(s++) - '0';
  *end = s;
  return num;
}

int strtoh(char *s, char **end) {
  int num = 0;
  int aux;
  while (isNumber(*s) || isHex(*s)) {
    aux = toLower(*s);
    num =
        num * 16 + isNumber(aux) * (aux - '0') + isHex(aux) * (aux - 'a' + 10);
    s++;
  }
  *end = s;
  return num;
}

void *my_malloc(uint64_t size) { return (void *)sys_malloc(size); }

void my_free(void *ptr) { sys_free(ptr); }

int64_t my_create_process(entry_point_t main, char **argv, char *name,
                          fd_t *file_descriptors) {
  return sys_create_process(main, argv, name, file_descriptors);
}

uint64_t my_exit(int exit_code) { return sys_exit_process(exit_code); }

uint64_t my_yield() { return sys_yield(); }

int64_t my_getpid() { return sys_getpid(); }

int64_t my_block_process(int64_t pid) { return sys_block_process(pid); }

int64_t my_unblock_process(int64_t pid) { return sys_unblock_process(pid); }

int64_t my_nice(int64_t pid, uint8_t priority) {
  return sys_set_priority(pid, priority);
}

int64_t my_kill(int64_t pid) { return sys_kill_process(pid); }

int64_t my_wait_pid(int64_t pid, int *exit_code) {
  return sys_wait_pid(pid, exit_code);
}

int64_t my_sem_open(char *name, uint64_t initialValue) {
  return sys_sem_open(name, initialValue);
}

int64_t my_sem_wait(char *name) { return sys_sem_wait(name); }

int64_t my_sem_post(char *name) { return sys_sem_post(name); }

int64_t my_sem_close(char *name) { return sys_sem_close(name); }

process_info_t *my_get_processes_info() { return sys_get_processes_info(); }

int64_t my_total_cpu_ticks() { return sys_total_cpu_ticks(); }

memory_info_t *my_memory_get_info() { return sys_memory_get_info(); }

void my_sleep(uint64_t seconds) { sys_sleep(seconds); }

int my_pipe_create(fd_t fds[2]){
    return sys_create_pipe(fds);
}

int my_pipe_write(fd_t fd, const char * buffer, int size){
    return sys_write_pipe(fd, buffer, size);
}

int my_pipe_read(fd_t fd, char * buffer, int size){
    return sys_read_pipe(fd, buffer, size);
}

void my_destroy_pipe(fd_t fd){
    sys_destroy_pipe(fd);
}

void my_adopt_child(int64_t pid){
    sys_adopt_child(pid);
}
