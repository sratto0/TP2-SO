#include "../include/shellCommands.h"
#include "../include/stdlib.h"
#include "../include/stdio.h"
#include <stddef.h>


int cmd_ps(int argc, char **argv) {
    if (argc != 1) {
        printf("ps: usage: ps\n");
        return -1;
    }
    process_info_t *info = my_get_processes_info();
    if (info == NULL) {
        printf("ps: no data\n");
        return -1;
    }

    uint64_t total = my_total_cpu_ticks();
    printf("PID  PPID PR ST     FG  CPU%%  NAME\n");
    // for (process_info_t *p = info; p->pid != NO_PID; p++) {
    //     uint64_t cpu = (total == 0) ? 0 : (p->ticks * 100) / total;
    //     printf("%d  %d %d %s    %d  %d   %s\n",
    //            (int)p->pid, (int)p->parent_pid, p->priority,
    //            state_to_str(p->state), p->foreground,
    //            (unsigned long long)cpu, p->name);
    // }

    for (process_info_t *p = info; p != NULL; p++) {
        uint64_t cpu = (total == 0) ? 0 : (p->ticks * 100) / total;
        printf("%d  %d %d %s    %d  %d   %s\n",
               (int)p->pid, (int)p->parent_pid, p->priority,
               state_to_str(p->state), p->foreground,
               (unsigned long long)cpu, p->name);
    }
    return 0;
}

int cmd_mem(int argc, char **argv) {
    if (argc != 1) {
        printf("mem: usage: mem\n");
        return -1;
    }
    memory_info_t *info = my_memory_get_info();
    if (info == NULL) {
        printf("mem: no data\n");
        return -1;
    }

    printf("Total Memory: %d bytes\n", (unsigned long long)info->size);
    printf("Used Memory:  %d bytes\n", (unsigned long long)info->used);
    printf("Free Memory:  %d bytes\n", (unsigned long long)info->free);
    return 0;
}



