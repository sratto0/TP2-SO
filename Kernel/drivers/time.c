// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "./../include/time.h"
#include "memoryManager.h"
#include <stdint.h>

static uint64_t ticks = 0;
static sleeping_process_t *sleeping_processes = NULL;
static uint64_t sleeping_capacity = 0;
static uint64_t next_tick = UINT64_MAX;
static void unblock_sleeping_processes();
static int ensure_sleeping_capacity(uint64_t pid_index);

uint64_t timerHandler(uint64_t rsp) {
  ticks++;
  unblock_sleeping_processes();
  rsp = (uint64_t)schedule((void *)rsp);
  return rsp;
}

uint64_t ticksElapsed() { return ticks; }

int secondsElapsed() { return ticks / 18; }

void init_sleeping_processes() {
  if (ensure_sleeping_capacity(INITIAL_PROCESS_CAPACITY - 1) != 0) {
    sleeping_capacity = 0;
    sleeping_processes = NULL;
  }
  if (sleeping_processes != NULL) {
    for (uint64_t i = 0; i < sleeping_capacity; i++) {
      sleeping_processes[i].pid = NO_PID;
      sleeping_processes[i].wake_up_tick = 0;
    }
  }
  next_tick = UINT64_MAX;
}

void remove_sleeping_process(int64_t pid) {
  if (pid < 0 || sleeping_processes == NULL ||
      (uint64_t)pid >= sleeping_capacity) {
    return;
  }
  sleeping_processes[pid].pid = NO_PID;
}

static void unblock_sleeping_processes() {
  if (ticks < next_tick)
    return;
  next_tick = UINT64_MAX;
  if (sleeping_processes == NULL) {
    return;
  }
  for (uint64_t i = 0; i < sleeping_capacity; i++) {
    if (sleeping_processes[i].pid != NO_PID &&
        sleeping_processes[i].wake_up_tick <= ticks) {
      unblock_process(sleeping_processes[i].pid);
      sleeping_processes[i].pid = NO_PID;
    } else if (sleeping_processes[i].pid != NO_PID &&
               sleeping_processes[i].wake_up_tick < next_tick) {
      next_tick = sleeping_processes[i].wake_up_tick;
    }
  }
}

void sleep(uint32_t sleeping_ticks) {
  int64_t pid = get_current_pid();
  if (pid < 0) {
    return;
  }
  if (ensure_sleeping_capacity((uint64_t)pid) != 0) {
    return;
  }
  sleeping_processes[pid].wake_up_tick = ticks + sleeping_ticks;
  sleeping_processes[pid].pid = pid;
  if (sleeping_processes[pid].wake_up_tick < next_tick)
    next_tick = sleeping_processes[pid].wake_up_tick;
  block_current_process();
}

static int ensure_sleeping_capacity(uint64_t pid_index) {
  if (pid_index < sleeping_capacity && sleeping_processes != NULL) {
    return 0;
  }

  uint64_t new_capacity =
      (sleeping_capacity == 0) ? INITIAL_PROCESS_CAPACITY : sleeping_capacity;
  while (pid_index >= new_capacity) {
    new_capacity *= 2;
  }

  sleeping_process_t *new_array =
      memory_alloc(sizeof(sleeping_process_t) * new_capacity);
  if (new_array == NULL) {
    return -1;
  }

  for (uint64_t i = 0; i < new_capacity; i++) {
    new_array[i].pid = NO_PID;
    new_array[i].wake_up_tick = 0;
  }

  if (sleeping_processes != NULL) {
    for (uint64_t i = 0; i < sleeping_capacity; i++) {
      if (sleeping_processes[i].pid != NO_PID) {
        new_array[i] = sleeping_processes[i];
      }
    }
    memory_free(sleeping_processes);
  }

  sleeping_processes = new_array;
  sleeping_capacity = new_capacity;
  return 0;
}
