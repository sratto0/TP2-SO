// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "memory.h"
#include "memoryManager.h"
#include "process.h"
#include "scheduler.h"
#include "semaphore.h"
#include "stdlib.h"
#include <color.h>
#include <keyboard.h>
#include <lib.h>
#include <stdint.h>
#include <time.h>
#include <video.h>

/* File Descriptors*/
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define KBDIN 3

/* IDs de syscalls */
#define READ 0
#define WRITE 1
#define CLEAR 2
#define SECONDS 3
#define GET_REGISTER_ARRAY 4
#define SET_FONT_SIZE 5
#define GET_RESOLUTION 6
#define DRAW_RECT 7
#define GET_TICKS 8
#define GET_MEMORY 9
#define SET_FONT_COLOR 10
#define GET_FONT_COLOR 11
#define MALLOC 12
#define FREE 13
#define CREATE_PROCESS 14
#define EXIT_PROCESS 15
#define YIELD 16
#define GET_PID 17
#define BLOCK_PROCESS 18
#define UNBLOCK_PROCESS 19
#define SET_PRIORITY 20
#define GET_PROCESSES_INFO 21
#define KILL_PROCESS 22
#define WAIT_PID 23
#define TOTAL_CPU_TICKS 24
#define SEM_OPEN 25
#define SEM_WAIT 26
#define SEM_POST 27
#define SEM_CLOSE 28
#define MEMORY_INFO 29
#define SLEEP 30
#define READ_STDIN 31
#define WRITE_STDOUT 32

static uint8_t syscall_read(uint32_t fd);
static void syscall_write(uint32_t fd, char c);
static void syscall_clear();
static uint32_t syscall_seconds();
static uint64_t *syscall_registerArray(uint64_t *regarr);
static void syscall_fontSize(uint8_t size);
static uint32_t syscall_resolution();
static uint64_t syscall_getTicks();
static void syscall_getMemory(uint64_t pos, uint8_t *vec);
static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b);
static uint32_t syscall_getFontColor();
static uint64_t syscall_malloc(uint64_t size);
static void syscall_free(void *ptr);
static int64_t syscall_create_process(entry_point_t main, char **argv,
                                      char *name, int *file_descriptors);
static void syscall_exit_process(int64_t exit_code);
static void syscall_yield();
static int64_t syscall_get_pid();
static int syscall_block_process(int64_t pid);
static int syscall_unblock_process(int64_t pid);
static int syscall_set_priority(int64_t pid, uint8_t priority);
static uint64_t syscall_get_processes_info();
static int syscall_kill_process(int64_t pid);
static int64_t syscall_wait_pid(int64_t pid, int32_t *exit_code);
static uint64_t syscall_total_ticks();
static int64_t syscall_sem_open(char *name, uint64_t initialValue);
static int64_t syscall_sem_wait(char *name);
static int64_t syscall_sem_post(char *name);
static int64_t syscall_sem_close(char *name);
static uint64_t syscall_memeory_get_info();
static void syscall_sleep(uint64_t seconds);
static int syscall_read_stdin(char *buffer, uint64_t size);
static int syscall_write_stdout(char *buffer, uint64_t size);

uint64_t syscallDispatcher(uint64_t nr, uint64_t arg0, uint64_t arg1,
                           uint64_t arg2, uint64_t arg3, uint64_t arg4,
                           uint64_t arg5) {
  switch (nr) {
  case READ:
    return syscall_read((uint32_t)arg0);
  case WRITE:
    syscall_write((uint32_t)arg0, (char)arg1);
    break;
  case CLEAR:
    syscall_clear();
    break;
  case SECONDS:
    return syscall_seconds();
  case GET_REGISTER_ARRAY:
    return (uint64_t)syscall_registerArray((uint64_t *)arg0);
  case SET_FONT_SIZE:
    syscall_fontSize((uint8_t)arg0);
    break;
  case GET_RESOLUTION:
    return syscall_resolution();
  case GET_TICKS:
    return syscall_getTicks();
  case GET_MEMORY:
    syscall_getMemory((uint64_t)arg0, (uint8_t *)arg1);
    break;
  case SET_FONT_COLOR:
    syscall_setFontColor((uint8_t)arg0, (uint8_t)arg1, (uint8_t)arg2);
    break;
  case GET_FONT_COLOR:
    return syscall_getFontColor();
  case MALLOC:
    return (uint64_t)syscall_malloc((uint64_t)arg0);
    break;
  case FREE:
    syscall_free((void *)arg0);
    break;
  case CREATE_PROCESS:
    return (uint64_t)syscall_create_process((entry_point_t)arg0, (char **)arg1,
                                            (char *)arg2, (int *)arg3);
    break;
  case EXIT_PROCESS:
    syscall_exit_process((int64_t)arg0);
    break;
  case YIELD:
    syscall_yield();
    break;
  case GET_PID:
    return (uint64_t)syscall_get_pid();
  case BLOCK_PROCESS:
    return (uint64_t)syscall_block_process((int64_t)arg0);
  case UNBLOCK_PROCESS:
    return (uint64_t)syscall_unblock_process((int64_t)arg0);
  case SET_PRIORITY:
    return (uint64_t)syscall_set_priority((int64_t)arg0, (uint8_t)arg1);
  case GET_PROCESSES_INFO:
    return (uint64_t)syscall_get_processes_info();
  case KILL_PROCESS:
    return (uint64_t)syscall_kill_process((int64_t)arg0);
  case WAIT_PID:
    return (uint64_t)syscall_wait_pid((int64_t)arg0, (int32_t *)arg1);
  case TOTAL_CPU_TICKS:
    return (uint64_t)syscall_total_ticks();
  case SEM_OPEN:
    return (uint64_t)syscall_sem_open((char *)arg0, (uint64_t)arg1);
  case SEM_WAIT:
    return (uint64_t)syscall_sem_wait((char *)arg0);
  case SEM_POST:
    return (uint64_t)syscall_sem_post((char *)arg0);
  case SEM_CLOSE:
    return (uint64_t)syscall_sem_close((char *)arg0);
  case MEMORY_INFO:
    return (uint64_t)syscall_memeory_get_info();
  case SLEEP:
    syscall_sleep((uint64_t)arg0);
    break;
  case READ_STDIN:
    return (uint64_t)syscall_read_stdin((char *)arg0, (uint64_t)arg1);
  case WRITE_STDOUT:
    return (uint64_t)syscall_write_stdout((char *)arg0, (uint64_t)arg1);
  }
  return 0;
}

// Read char
static uint8_t syscall_read(uint32_t fd) {
  switch (fd) {
  case STDIN:
    return getAscii();
  case KBDIN:
    return getScancode();
  }
  return 0;
}

// Write char
static void syscall_write(uint32_t fd, char c) {
  Color prevColor = getFontColor();
  if (fd == STDERR)
    setFontColor(ERROR_COLOR);
  else if (fd != STDOUT)
    return;
  printChar(c);
  setFontColor(prevColor);
}

// Clear
static void syscall_clear() { videoClear(); }

// Get time in seconds
static uint32_t syscall_seconds() {
  uint8_t h, m, s;
  getTime(&h, &m, &s);
  return s + m * 60 + ((h + 24 - 3) % 24) * 3600;
}

// Get register snapshot array
static uint64_t *syscall_registerArray(uint64_t *regarr) {
  uint64_t *snapshot = getLastRegSnapshot();
  for (int i = 0; i < QTY_REGS; i++)
    regarr[i] = snapshot[i];
  return regarr;
}

// Set fontsize
static void syscall_fontSize(uint8_t size) { setFontSize(size - 1); }

// Get screen resolution
static uint32_t syscall_resolution() { return getScreenResolution(); }

// GetTicks
static uint64_t syscall_getTicks() { return ticksElapsed(); }

// PrintMem
static void syscall_getMemory(uint64_t pos, uint8_t *vec) {
  memcpy(vec, (uint8_t *)pos, 32);
}

// Set fontsize
static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b) {
  setFontColor((Color){b, g, r});
}

// Get fontsize
static uint32_t syscall_getFontColor() {
  ColorInt c = {color : getFontColor()};
  return c.bits;
}

// Malloc
static uint64_t syscall_malloc(uint64_t size) {
  return (uint64_t)memory_alloc(size);
}

// Free
static void syscall_free(void *ptr) { memory_free(ptr); }

// Create process
static int64_t syscall_create_process(entry_point_t main, char **argv,
                                      char *name, int *file_descriptors) {
  return add_process((entry_point_t)main, argv, name, file_descriptors);
}

// Exit process
static void syscall_exit_process(int64_t exit_code) { my_exit(exit_code); }

// Yield
static void syscall_yield() { yield(); }

// Get PID
static int64_t syscall_get_pid() { return get_current_pid(); }

// Block process
static int syscall_block_process(int64_t pid) { return block_process(pid); }

// Unblock process
static int syscall_unblock_process(int64_t pid) { return unblock_process(pid); }

// Set priority
static int syscall_set_priority(int64_t pid, uint8_t priority) {
  return set_process_priority(pid, priority);
}

// Get process info
static uint64_t syscall_get_processes_info() {
  return (uint64_t)get_processes_info();
}

// Kill process
static int syscall_kill_process(int64_t pid) { return kill_process(pid); }

// Wait PID
static int64_t syscall_wait_pid(int64_t pid, int32_t *exit_code) {
  return wait_pid(pid, exit_code);
}

// Total CPU ticks
static uint64_t syscall_total_ticks() { return total_ticks(); }

// Semaphore open
static int64_t syscall_sem_open(char *name, uint64_t initialValue) {
  return my_sem_open(name, initialValue);
}

// Semaphore wait
static int64_t syscall_sem_wait(char *name) { return my_sem_wait(name); }

// Semaphore post
static int64_t syscall_sem_post(char *name) { return my_sem_post(name); }

// Semaphore close
static int64_t syscall_sem_close(char *name) { return my_sem_close(name); }

static uint64_t syscall_memeory_get_info() {
  return (uint64_t)memory_get_info();
}

static void syscall_sleep(uint64_t seconds) {
  uint32_t sleeping_ticks = (uint32_t)(seconds * 18);
  sleep(sleeping_ticks);
}

static int syscall_read_stdin(char *buffer, uint64_t size) {
  uint64_t bytesRead = 0;
  while (bytesRead < size) {
    char c = syscall_read(STDIN);
    if (c == '\n' || c == '\r') {
      buffer[bytesRead++] = '\n';
      break;
    }
    buffer[bytesRead++] = c;
  }
  return (int)bytesRead;
}

static int syscall_write_stdout(char *buffer, uint64_t size) {
  uint64_t bytesWritten = 0;
  while (bytesWritten < size) {
    syscall_write(STDOUT, buffer[bytesWritten++]);
  }
  return (int)bytesWritten;
}