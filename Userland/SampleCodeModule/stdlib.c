// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include "../../SharedLibraries/sharedStructs.h"


#define toLower(n) ((n) >= 'A' && (n) <= 'Z' ? (n) - ('A' - 'a') : (n))
#define isNumber(n) ((n) >= '0' && (n) <= '9')
#define isHex(n) ((n) >= 'a' && (n) <= 'f')

static int calculate_total_ticks(process_info_t * process_array);

static unsigned int log(uint64_t n, int base) {
    unsigned int count = 1;
    while (n /= base)
        count++;
    return count;
}

int itoa(uint64_t n, char* buffer, int base)
{
 
    if (n == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }
 
    unsigned int len = 0;
    int i = 0;
    if (n < 0 && base == 10)
    {
        n = -n;
        buffer[i] = '-';
        len++;
        i++;
    }
 
    len += log(n, base);
    while (n != 0)
    {
        int r = n % base;
        buffer[len - i++ - 1] = (r > 9)? (r-10) + 'A' : r + '0';
        n /= base;
    }
    buffer[i] = '\0'; 
    return len;
}

int atoi(char * s)
{
    int num = 0;
    while (isNumber(*s))
        num = num * 10 + *(s++) - '0';
    return num;
}

int strtoi(char* s, char ** end) {
    int num = 0;
    while (isNumber(*s))
        num = num * 10 + *(s++) - '0';
    *end = s; 
    return num;
}

int strtoh(char* s, char ** end) {
    int num = 0;
    int aux;
    while (isNumber(*s) || isHex(*s)) {
        aux = toLower(*s);
        num = num * 16 + isNumber(aux)*(aux - '0') + isHex(aux)*(aux - 'a' + 10);
        s++;
    }
    *end = s; 
    return num;
}

void * my_malloc(uint64_t size){
    return (void *) sys_malloc(size);
}

void my_free(void * ptr){
    sys_free(ptr);
}

// int ps(int argc, chat * argv[]) {
//     if (argc != 0) {
//         printf("ps: Invalid number of arguments.\n");
//         return -1;
//     }

//     process_info_t * process_array = (process_info_t *) sys_get_processes_info();
//     process_info_t * current  = process_array;
    
//     int total_cpu_ticks = calculate_total_ticks(process_array);
//     ftab_print(GRAY, "PID", 4);
//     ftab_print(GRAY, "PPID", 5);
//     ftab_print(GRAY, "Prio", 5);
//     ftab_print(GRAY, "Stat", 10);
//     ftab_print(GRAY, "Name", 8);
//     ftab_print(GRAY, "StackB", 9);
//     ftab_print(GRAY, "StackP", 10);
//     ftab_print(GRAY, "FG", 5);
//     ftab_print(GRAY, "CPU%", 10);
//     putchar('\n');

    
//     while(current->pid != NO_PID) {
//         int CPU_percent = (total_cpu_ticks > 0) ? (current->ticks * 100) / total_cpu_ticks : 0;
        
//         if(current->p_pid < 0) {
//             ftab_print("-", 5)
//         }
        
//     }
// }

// static void ftab_print(int fd, char * buffer, int width) {
//     int len = 0;
//     const char * str = buffer;
//     while(*str++) len++;
//     fd_print(fd, "%s", buffer);
//     for(int i = 0; i < width - len; i++) putchar(' ');
// }


// int fd_print(uint64_t fd, const char * format, ...) {
//     va_list args;
//     va_start(args, format);
//     int toRet = print_args(fd, format, args);
//     va_end(args);
//     return toRet;
// }

// static int print_args(uint64_t fd, const char * format, va_list args) {
//     char buf[BUFFER_SIZE];
    
// }

static void print_int(int val, int width){
    int len = 0, tmp = (val < 0) ? -val : val;
    if(val == 0) len = 1;
    else{
        while(tmp){
            len++;
            tmp /= 10;
        }
    }
    if(val < 0) len++;
    printf("%d", val);
    for(int i = 0; i < width - len; i++) putchar(' ');
}

static int calculate_total_ticks(process_info_t * process_array){
    int total = 0;
    process_info_t * current = process_array;
    while (current->pid != NO_PID) {
        total += current->ticks;
        current++;
    }
    return total;
}

int64_t my_create_process(uint64_t main, char ** argv, char * name, uint8_t no_kill, int * file_descriptors) {
    return sys_create_process(main, argv, name, no_kill, file_descriptors);
}

uint64_t my_exit(int exit_code) {
    return sys_exit_process(exit_code);
}

uint64_t my_yield() {
    return sys_yield();
}

int64_t my_getpid() {
    return sys_getpid();
}

int64_t my_block_process(int64_t pid) {
    return sys_block_process(pid);
}

int64_t my_unblock_process(int64_t pid) {
    return sys_unblock_process(pid);
}

int64_t my_nice(int64_t pid, uint8_t priority) {
    return sys_set_priority(pid, priority);
}

int64_t my_kill(int64_t pid) {
    return sys_kill_process(pid);
}

int64_t my_wait_pid(int64_t pid, int * exit_code) {
    return sys_wait_pid(pid, exit_code);
}
