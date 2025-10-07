// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <stdint.h>
#include <video.h>
#include <keyboard.h>
#include <lib.h>
#include <color.h>
#include <time.h>
#include "memory.h"
#include "stdlib.h"
#include "memoryManager.h"
#include "scheduler.h"

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
#define PROCESS_INFO 15
#define GETPID 16
#define KILL 17
#define CHANGE_PRIORITY 18
#define BLOCK 19
#define UNBLOCK 20
#define NICE 21

static uint8_t syscall_read(uint32_t fd);
static void syscall_write(uint32_t fd, char c);
static void syscall_clear();
static uint32_t syscall_seconds();
static uint64_t * syscall_registerArray(uint64_t * regarr);
static void syscall_fontSize(uint8_t size);
static uint32_t syscall_resolution();
static uint64_t syscall_getTicks();
static void syscall_getMemory(uint64_t pos, uint8_t * vec);
static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b);
static uint32_t syscall_getFontColor();
static uint64_t syscall_malloc(uint64_t size);  
static void syscall_free(void * ptr);
static uint64_t syscall_create_process(char *name, uint64_t argc, char *argv[]);
static uint64_t syscall_process_info(int32_t pid, process_t *out);
static uint64_t syscall_getpid();
static uint64_t syscall_kill(uint64_t pid);
static void syscall_change_priority(process_t * p, uint8_t new_prio);
static uint64_t syscall_block(uint64_t pid);
static uint64_t syscall_unblock(uint64_t pid);
static uint64_t syscall_nice(uint64_t pid, uint64_t new_prio);        


uint64_t syscallDispatcher(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
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
            return (uint64_t) syscall_registerArray((uint64_t *) arg0);
        case SET_FONT_SIZE:
            syscall_fontSize((uint8_t)arg0);
            break;
        case GET_RESOLUTION:
            return syscall_resolution();
        case GET_TICKS:
            return syscall_getTicks();
        case GET_MEMORY:
            syscall_getMemory((uint64_t) arg0, (uint8_t *) arg1);
            break;
        case SET_FONT_COLOR:
            syscall_setFontColor((uint8_t) arg0, (uint8_t) arg1, (uint8_t) arg2);
            break;
        case GET_FONT_COLOR:
            return syscall_getFontColor();
        case MALLOC:
            return (uint64_t) syscall_malloc((uint64_t) arg0);
            break;
        case FREE:
            syscall_free((void *) arg0);
            break;
        case CREATE_PROCESS:
            return (uint64_t) syscall_create_process((char *) arg0, (uint64_t) arg1, (char **) arg2);
            break;
        case PROCESS_INFO:
            return (uint64_t) syscall_process_info((int32_t) arg0, (process_t *) arg1);
            break;
        case GETPID:
            return (uint64_t) syscall_getpid();
            break;
        case KILL:
            return (uint64_t) syscall_kill((uint64_t) arg0);
            break;
        case CHANGE_PRIORITY:
            syscall_change_priority((process_t *) arg0, (uint8_t) arg1);
            break;
        case BLOCK:
            return (uint64_t) syscall_block((uint64_t) arg0);
            break;
        case UNBLOCK:
            return (uint64_t) syscall_unblock((uint64_t) arg0);
            break;
        case NICE:
            return (uint64_t) syscall_nice((uint64_t) arg0, (uint64_t) arg1);
            break;
	}
	return 0;
}

// Read char
static uint8_t syscall_read(uint32_t fd){
    switch (fd){
        case STDIN:
            return getAscii();
        case KBDIN:
            return getScancode();
    }
    return 0;
}

// Write char
static void syscall_write(uint32_t fd, char c){
    Color prevColor = getFontColor();
    if(fd == STDERR)
        setFontColor(ERROR_COLOR);
    else if(fd != STDOUT)
        return;
    printChar(c);
    setFontColor(prevColor);
}

// Clear
static void syscall_clear(){
    videoClear();
}

// Get time in seconds
static uint32_t syscall_seconds(){
    uint8_t h, m, s;
    getTime(&h, &m, &s);
    return s + m * 60 + ((h + 24 - 3) % 24) * 3600;
}

// Get register snapshot array
static uint64_t * syscall_registerArray(uint64_t * regarr){
    uint64_t * snapshot = getLastRegSnapshot();
    for(int i = 0; i < QTY_REGS; i++)
        regarr[i] = snapshot[i];
    return regarr;
}

// Set fontsize
static void syscall_fontSize(uint8_t size){
    setFontSize(size - 1);
}

// Get screen resolution
static uint32_t syscall_resolution(){
    return getScreenResolution();
}

// GetTicks
static uint64_t syscall_getTicks(){
    return ticksElapsed();
}

//PrintMem
static void syscall_getMemory(uint64_t pos, uint8_t * vec){
    memcpy(vec, (uint8_t *) pos, 32);
}

//Set fontsize
static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b){
    setFontColor((Color){b, g, r});
}

//Get fontsize
static uint32_t syscall_getFontColor(){
    ColorInt c = { color: getFontColor() };
    return c.bits;
}

//Malloc
static uint64_t syscall_malloc(uint64_t size){
    return (uint64_t) memory_alloc(size);
}

//Free
static void syscall_free(void * ptr){
    memory_free(ptr);
}

//Create process
static uint64_t syscall_create_process(char *name, uint64_t argc, char *argv[]){
    return (uint64_t) my_create_process(name, argc, argv);
}

//Process info
static uint64_t syscall_process_info(int32_t pid, process_t *out){
    return (uint64_t) my_process_info(pid, out);
}

//Get pid
static uint64_t syscall_getpid(){
    return (uint64_t) my_getpid();
}

//Kill
static uint64_t syscall_kill(uint64_t pid){
    return (uint64_t) my_kill(pid);
}

//Change priority
static void syscall_change_priority(process_t * p, uint8_t new_prio){
    my_change_priority(p, new_prio);
}

//Block
static uint64_t syscall_block(uint64_t pid){
    return (uint64_t) my_block(pid);
}

//Unblock
static uint64_t syscall_unblock(uint64_t pid){
    return (uint64_t) my_unblock(pid);
}

//Nice
static uint64_t syscall_nice(uint64_t pid, uint64_t new_prio){
    return (uint64_t) my_nice(pid, new_prio);
}