#include <stdio.h>
#include <string.h>

static process_t proc_table[MAX_PROCESSES];
static int32_t next_pid = 1;

void process_system_init(void) {
  memset(proc_table, 0, sizeof(proc_table));
  next_pid = 1;
  scheduler_init();
}

/* Crea un proceso: devuelve pid (>0) o -1 si no hay lugar */
int32_t my_create_process(const char *name, int priority, char *argv[]) {
  (void)argv;

  extern void *set_stack_frame(void *arg1, void *arg2, void *stack_top, void *arg4);
  const size_t STACK_SIZE = 0x4000; /* 16 KiB per process stack */

  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == 0 || proc_table[i].state == PROC_KILLED) {
      int32_t pid = next_pid++;
      proc_table[i].pid = pid;
    //   strncpy(proc_table[i].name, name ? name : "proc", sizeof(proc_table[i].name)-1);
      proc_table[i].name[sizeof(proc_table[i].name)-1] = '\0';
      proc_table[i].priority = (priority < 0) ? 0 : (priority >= PRIORITY_LEVELS ? (PRIORITY_LEVELS - 1) : (uint8_t)priority);
      proc_table[i].state = PROC_READY;
      proc_table[i].stack_pointer = NULL;
      proc_table[i].stack_base = NULL;

      /* Allocate stack memory from kernel allocator */
      void *stack_mem = memory_alloc(STACK_SIZE);
      if (!stack_mem) {
        /* allocation failed */
        proc_table[i].pid = 0;
        return -1;
      }
      /* compute aligned top of stack (stack grows down) */
      void *stack_top = (void *)((uintptr_t)stack_mem + STACK_SIZE);
      stack_top = (void *)((uintptr_t)stack_top & ~0xFULL); /* align to 16 bytes */

      /* set_stack_frame(arg1=NULL, arg2=NULL, stack_top, arg4=NULL) */
      void *frame = set_stack_frame(NULL, NULL, stack_top, NULL);
      if (!frame) {
        memory_free(stack_mem);
        proc_table[i].pid = 0;
        return -1;
      }

      proc_table[i].stack_base = stack_mem;
      proc_table[i].stack_pointer = frame;

      scheduler_add(&proc_table[i]);
      return pid;
    }
  }
  return -1;
}

int my_kill(int32_t pid) {
  if (pid <= 0) return -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == pid && proc_table[i].state != PROC_KILLED) {
      proc_table[i].state = PROC_KILLED;
      scheduler_remove(&proc_table[i]);
      /* Si es el proceso actualmente en CPU, forzar yield para cambiar */
      process_t *cur = scheduler_current();
      if (cur && cur->pid == pid) scheduler_yield();
      return 0;
    }
  }
  return -1;
}

/* Bloquea un proceso (acepta READY o RUNNING) */
int my_block(int32_t pid) {
  if (pid <= 0) return -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == pid) {
      if (proc_table[i].state == PROC_BLOCKED || proc_table[i].state == PROC_KILLED) return -1;
      proc_table[i].state = PROC_BLOCKED;
      scheduler_remove(&proc_table[i]);
      /* Si era el proceso en ejecución, cambiar CPU */
      process_t *cur = scheduler_current();
      if (cur && cur->pid == pid) scheduler_yield();
      return 0;
    }
  }
  return -1;
}

int my_unblock(int32_t pid) {
  if (pid <= 0) return -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == pid) {
      if (proc_table[i].state != PROC_BLOCKED) return -1;
      proc_table[i].state = PROC_READY;
      scheduler_add(&proc_table[i]);
      return 0;
    }
  }
  return -1;
}

int my_nice(int32_t pid, int new_prio) {
  if (pid <= 0) return -1;
  if (new_prio < 0) new_prio = 0;
  if (new_prio >= PRIORITY_LEVELS) new_prio = PRIORITY_LEVELS - 1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == pid && proc_table[i].state != PROC_KILLED) {
      my_change_priority(&proc_table[i], (uint8_t)new_prio);
      return 0;
    }
  }
  return -1;
}

int32_t my_getpid(void) {
  process_t *c = scheduler_current();
  return c ? c->pid : -1;
}

void my_list_processes(void) {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid != 0 && proc_table[i].state != PROC_KILLED) {
      printf("PID=%d NAME=%s PRIO=%d STATE=%d\n",
             proc_table[i].pid,
             proc_table[i].name,
             proc_table[i].priority,
             proc_table[i].state);
    }
  }
}

int my_process_info(int32_t pid, process_t *out) {
  if (pid <= 0) return -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].pid == pid) {
      process_t info;
      info.pid = proc_table[i].pid;
    //   strncpy(info.name, proc_table[i].name, sizeof(info.name)-1);
      info.name[sizeof(info.name)-1] = '\0';
      info.priority = proc_table[i].priority;
      info.state = proc_table[i].state;
      info.stack_pointer = proc_table[i].stack_pointer;
    //   info.stack_base = (void *)&proc_table[i].stack[0];
    //   info.stack_top = (void *)&proc_table[i].stack[STACK_SIZE];
      info.entry = proc_table[i].entry;
      info.arg = proc_table[i].arg;

      if (out) {
        *out = info;
        return 0;
      }

      /* imprimir si out == NULL */
      const char *state_str = "UNKNOWN";
      switch (info.state) {
        case PROC_READY: state_str = "READY"; break;
        case PROC_RUNNING: state_str = "RUNNING"; break;
        case PROC_BLOCKED: state_str = "BLOCKED"; break;
        case PROC_KILLED: state_str = "KILLED"; break;
        default: break;
      }

      printf("PID=%d NAME=%s PRIO=%d STATE=%s\n", info.pid, info.name, info.priority, state_str);
      printf("  stack_pointer=%p stack_base=%p stack_top=%p\n", info.stack_pointer, info.stack_base, info.stack_top);
      printf("  entry=%p arg=%p\n", (void*)info.entry, info.arg);
      return 0;
    }
  }
  return -1;
}
