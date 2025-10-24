#ifndef SCHEDULER_H
#define SCHEDULER_H
#define SCHEDULER_ADDRESS 0x60000
#define SHELL_PID 1

typedef struct schedulerCDT * schedulerADT; 

void scheduler_init();
schedulerADT get_scheduler();
uint16_t total_ticks();
void * schedule(void * context);
int64_t add_process(entry_point_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors);
static process_t * get_next_process();
void destroy_scheduler();
uint16_t get_current_pid();
void yield();
static void adopt_children(int16_t pid);
static void remove_process(uint16_t pid);
int kill_process(uint16_t pid);
int32_t kill_current_process();
int block_process(uint16_t pid);
int unblock_process(uint16_t pid);
int block_current_process();
int unblock_current_process();
int set_process_priority(uint16_t pid, uint8_t priority);
int64_t wait_pid(unit16_t pid, int32_t * exit_code);
int sleep_block(uint16_t pid, uint8_t sleep);
void my_exit(int64_t ret);
uint8_t foreground_process(uint16_t pid);
process_info_t * get_processes_info();

#endif // SCHEDULER_H